console.log('Server is starting...');
// Import required libraries
const express = require('express');
const bodyParser = require('body-parser');
const jwt = require('jsonwebtoken');
const mqtt = require('mqtt');
const sqlite3 = require('sqlite3').verbose();
const bcrypt = require('bcrypt');

// Initialize application
const app = express();
const port = 3000;


// Middleware setup
app.use(bodyParser.json());

// Secret for JWT
const JWT_SECRET = 'your_secret_key';

// Initialize SQLite database
const db = new sqlite3.Database('./iot.db', (err) => {
    if (err) {
        console.error('Failed to connect to SQLite database:', err);
    } else {
        console.log('Connected to SQLite database.');
        initializeDatabase();
    }
});

// MQTT Client setup
const mqttClient = mqtt.connect('mqtt://localhost');

mqttClient.on('connect', () => {
    console.log('Connected to MQTT broker');
    mqttClient.subscribe('iot/+/data', (err) => {
        if (err) console.error('Failed to subscribe to topic:', err);
    });
});

mqttClient.on('message', (topic, message) => {
    const [_, deviceId, _type] = topic.split('/');
    const data = JSON.parse(message.toString());

    db.run(
        'INSERT INTO TelemetryData (deviceId, timestamp, sensorType, value) VALUES (?, ?, ?, ?)',
        [deviceId, new Date().toISOString(), data.type, data.value],
        (err) => {
            if (err) console.error('Failed to save telemetry data:', err);
        }
    );
});

// Database initialization
function initializeDatabase() {
    db.run(`CREATE TABLE IF NOT EXISTS Users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        email TEXT UNIQUE,
        password TEXT
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS IoTDevices (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT,
        macAddress TEXT UNIQUE,
        userId INTEGER,
        FOREIGN KEY(userId) REFERENCES Users(id)
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS TelemetryData (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        deviceId INTEGER,
        timestamp TEXT,
        sensorType TEXT,
        value REAL,
        FOREIGN KEY(deviceId) REFERENCES IoTDevices(id)
    )`);
}

// Helper functions
const authenticate = (req, res, next) => {
    const token = req.headers['authorization'];
    if (!token) return res.status(401).send('Access Denied');

    try {
        const verified = jwt.verify(token.split(' ')[1], JWT_SECRET);
        req.user = verified;
        next();
    } catch (err) {
        res.status(400).send('Invalid Token');
    }
};

app.get('/', (req, res) => {
    res.send('Hello, IoT!');
});


// API routes
app.post('/register', async (req, res) => {
    const { email, password } = req.body;
    const hashedPassword = await bcrypt.hash(password, 10);

    db.run(
        'INSERT INTO Users (email, password) VALUES (?, ?)',
        [email, hashedPassword],
        (err) => {
            if (err) {
                res.status(400).send('User already exists');
            } else {
                res.status(201).send('User registered');
            }
        }
    );
});

app.post('/login', (req, res) => {
    const { email, password } = req.body;

    db.get('SELECT * FROM Users WHERE email = ?', [email], async (err, user) => {
        if (err || !user) return res.status(400).send('Invalid credentials');

        const validPassword = await bcrypt.compare(password, user.password);
        if (!validPassword) return res.status(400).send('Invalid credentials');

        const token = jwt.sign({ id: user.id, email: user.email }, JWT_SECRET);
        res.header('auth-token', token).send({ token });
    });
});

app.post('/devices', authenticate, (req, res) => {
    const { name, macAddress } = req.body;

    db.run(
        'INSERT INTO IoTDevices (name, macAddress, userId) VALUES (?, ?, ?)',
        [name, macAddress, req.user.id],
        (err) => {
            if (err) {
                res.status(400).send('Device registration failed');
            } else {
                res.status(201).send('Device registered');
            }
        }
    );
});

app.get('/devices', authenticate, (req, res) => {
    db.all(
        'SELECT * FROM IoTDevices WHERE userId = ?',
        [req.user.id],
        (err, devices) => {
            if (err) {
                res.status(400).send('Failed to fetch devices');
            } else {
                res.send(devices);
            }
        }
    );
});

app.get('/telemetry/:deviceId', authenticate, (req, res) => {
    const { deviceId } = req.params;

    db.all(
        'SELECT * FROM TelemetryData WHERE deviceId = ?',
        [deviceId],
        (err, data) => {
            if (err) {
                res.status(400).send('Failed to fetch telemetry data');
            } else {
                res.send(data);
            }
        }
    );
});

// Start the server
app.listen(port, () => {
    console.log(`Server running on http://localhost:${port}`);
});


// Actions on device 
app.post('/devices/:deviceId/command', authenticate, (req, res) => {
    const { deviceId } = req.params;
    const { action, device } = req.body;

    if (!action || !device) {
        return res.status(400).send('Missing action or device in request');
    }

    const topic = `iot/${deviceId}/commands`;
    const message = JSON.stringify({ action, device });

    mqttClient.publish(topic, message, (err) => {
        if (err) {
            console.error('Failed to send command:', err);
            return res.status(500).send('Failed to send command');
        }

        console.log(`Command sent to device ${deviceId}:`, message);
        res.status(200).send(`Command sent to device ${deviceId}`);
    });
});
