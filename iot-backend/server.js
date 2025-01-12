console.log('Server is starting...');
// Import required libraries
const express = require('express');
const bodyParser = require('body-parser');
const jwt = require('jsonwebtoken');
const mqtt = require('mqtt');
const sqlite3 = require('sqlite3').verbose();
const bcrypt = require('bcrypt');
const cors = require('cors');


// Initialize application
const app = express();
const port = 4000;


// Middleware CORS
app.use(cors());


// Middleware setup
app.use(bodyParser.json());

app.use(cors({
    origin: 'http://localhost:5173', // Adres Twojego frontendu
  }));
  

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
    wifiId INTEGER,
    userId INTEGER,
    FOREIGN KEY(userId) REFERENCES Users(id),
    FOREIGN KEY(wifiId) REFERENCES WiFiNetworks(id)
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS WiFiNetworks (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        ssid TEXT NOT NULL,
        password TEXT NOT NULL,
        userId INTEGER NOT NULL,
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
    console.log('Request received:', req.body); // Dodaj logowanie

    const { email, password } = req.body;
    const hashedPassword = await bcrypt.hash(password, 10);

    db.run(
        'INSERT INTO Users (email, password) VALUES (?, ?)',
        [email, hashedPassword],
        (err) => {
            if (err) {
                console.error('Error inserting user:', err); // Logowanie błędów
                res.status(400).send('User already exists');
            } else {
                console.log('User registered:', email); // Logowanie sukcesu
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
    const { name, macAddress, wifiId } = req.body;

    if (!name || !macAddress || !wifiId) {
        return res.status(400).send('Name, MAC address, and WiFi ID are required');
    }

    const userId = req.user.id; // Extract user ID from the authenticated request

    db.run(
        'INSERT INTO IoTDevices (name, macAddress, wifiId, userId) VALUES (?, ?, ?, ?)',
        [name, macAddress, wifiId, userId],
        function (err) {
            if (err) {
                console.error('Failed to register device:', err);
                return res.status(500).send('Failed to register device');
            }

            res.status(201).send({ id: this.lastID, name, macAddress });
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

app.get('/wifi/:wifiId/devices', authenticate, (req, res) => {
    const { wifiId } = req.params;

    db.all(
        'SELECT * FROM IoTDevices WHERE wifiId = ?',
        [wifiId],
        (err, devices) => {
            if (err) {
                console.error('Failed to fetch devices:', err);
                return res.status(500).send('Failed to fetch devices');
            }

            res.send(devices);
        }
    );
});


app.post('/wifi', authenticate, (req, res) => {
    const { ssid, password } = req.body;

    if (!ssid || !password) {
        return res.status(400).send('SSID and password are required');
    }

    db.run(
        'INSERT INTO WiFiNetworks (ssid, password, userId) VALUES (?, ?, ?)',
        [ssid, password, req.user.id],
        function (err) {
            if (err) {
                console.error('Failed to add WiFi network:', err);
                return res.status(500).send('Failed to add WiFi network');
            }

            res.status(201).send({ id: this.lastID, ssid });
        }
    );
});

app.get('/wifi', authenticate, (req, res) => {
    console.log('Authorization header:', req.headers['authorization']);
    db.all(
        'SELECT id, ssid FROM WiFiNetworks WHERE userId = ?',
        [req.user.id],
        (err, networks) => {
            if (err) {
                console.error('Failed to fetch WiFi networks:', err);
                return res.status(500).send('Failed to fetch WiFi networks');
            }

            res.send(networks);
        }
    );
});




app.post('/devices/:deviceId/wifi', authenticate, (req, res) => {
    const { deviceId } = req.params;
    const { wifiId } = req.body;

    if (!wifiId) {
        return res.status(400).send('WiFi ID is required');
    }

    db.run(
        'UPDATE IoTDevices SET wifiId = ? WHERE id = ? AND userId = ?',
        [wifiId, deviceId, req.user.id],
        function (err) {
            if (err) {
                console.error('Failed to update device WiFi:', err);
                return res.status(500).send('Failed to update device WiFi');
            }

            if (this.changes === 0) {
                return res.status(404).send('Device not found or not owned by user');
            }

            res.status(200).send('Device WiFi updated');
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
