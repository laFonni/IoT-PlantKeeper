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
    mqttClient.subscribe('+/+', (err) => {
        if (err) console.error('Failed to subscribe to topic:', err);
    });
});

mqttClient.on('message', (topic, message) => {
    const [deviceMac, sensor_type] = topic.split('/');
    const data = JSON.parse(message.toString());

    console.log(`Received message on topic ${topic}:`, data);

    db.get(
        'SELECT id FROM IoTDevices WHERE macAddress = ?',
        [deviceMac],
        (err, row) => {
            if (err) {
                console.error('Failed to retrieve device ID:', err);
                return;
            }

            if (!row) {
                console.error('Device not found for MAC address:', deviceMac);
                return;
            }

            const deviceId = row.id;

            // Insert telemetry data into the database
            db.run(
                'INSERT INTO TelemetryData (deviceId, timestamp, sensorType, value) VALUES (?, ?, ?, ?)',
                [deviceId, new Date().toISOString(), sensor_type, data.value],
                (err) => {
                    if (err) console.error('Failed to save telemetry data:', err);
                }
            );

            // When the sensor type is "light_sensor", process the light sensor value and control the lamp
if (sensor_type === 'light_sensor') {
    db.get(
        'SELECT value FROM TelemetryData WHERE deviceId = ? AND sensorType = ? ORDER BY timestamp DESC LIMIT 1',
        [deviceId, 'light_sensor'],
        (err, row) => {
            if (err) {
                console.error('Failed to retrieve light sensor data:', err);
                return;
            }

            if (row) {
                const lightValue = row.value;
                const lampTopic = `${deviceMac}/lamp`;
                const lampState = lightValue < 2000 ? 1 : 0;

                // Publish the lamp state to the appropriate topic
                mqttClient.publish(lampTopic, lampState.toString(), (err) => {
                    if (err) {
                        console.error('Failed to publish lamp state:', err);
                    } else {
                        console.log(`Lamp state updated to ${lampState} on topic ${lampTopic}`);

                        // Save the lamp state as a new sensor record in the database
                        db.run(
                            'INSERT INTO TelemetryData (deviceId, timestamp, sensorType, value) VALUES (?, ?, ?, ?)',
                            [deviceId, new Date().toISOString(), 'light_emitter', lampState],
                            (err) => {
                                if (err) {
                                    console.error('Failed to save lamp state as sensor record:', err);
                                } else {
                                    console.log('Lamp state saved as light_emitter record in database.');
                                }
                            }
                        );
                    }
                });
            }
        }
    );
}
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

    db.run(`CREATE TABLE IF NOT EXISTS DeviceStatuses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    deviceId INTEGER NOT NULL,
    status TEXT NOT NULL,
    timestamp TEXT NOT NULL
    );`);

    db.run(`CREATE TABLE IF NOT EXISTS WiFiCredentials (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    deviceId INTEGER NOT NULL,
    ssid TEXT NOT NULL,
    password TEXT NOT NULL,
    timestamp TEXT NOT NULL
    );`);
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

app.get('/wifi', authenticate, (req, res) => {
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
app.delete('/wifi/:id', authenticate, (req, res) => {
    const { id } = req.params;

    db.run(
        'DELETE FROM WiFiNetworks WHERE id = ? AND userId = ?',
        [id, req.user.id],
        function (err) {
            if (err) {
                console.error('Failed to delete WiFi network:', err);
                return res.status(500).send('Failed to delete WiFi network');
            }

            if (this.changes === 0) {
                return res.status(404).send('WiFi network not found or not owned by user');
            }

            res.status(200).send('WiFi network deleted');
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

app.get('/wifi/:ssid', authenticate, (req, res) => {
    const { ssid } = req.params;

    db.get(
        'SELECT password FROM WiFiNetworks WHERE ssid = ? AND userId = ?',
        [ssid, req.user.id],
        (err, row) => {
            if (err) {
                console.error('Failed to fetch password:', err);
                return res.status(500).send('Failed to fetch password');
            }

            if (!row) {
                return res.status(404).send('Network not found');
            }

            res.send({ password: row.password });
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

app.post('/device-status', (req, res) => {
    const { deviceId, status } = req.body;

    if (!deviceId || !status) {
        return res.status(400).send('Device ID and status are required');
    }

    db.run(
        'INSERT INTO DeviceStatuses (deviceId, status, timestamp) VALUES (?, ?, ?)',
        [deviceId, status, new Date().toISOString()],
        (err) => {
            if (err) {
                console.error('Failed to log device status:', err);
                return res.status(500).send('Failed to log device status');
            }
            res.status(201).send('Status logged successfully');
        }
    );
});

app.post('/wifi-credentials', authenticate, (req, res) => {
    const { ssid, password, deviceId } = req.body;

    if (!ssid || !password || !deviceId) {
        return res.status(400).send('SSID, password, and device ID are required');
    }

    db.run(
        'INSERT INTO WiFiCredentials (deviceId, ssid, password, timestamp) VALUES (?, ?, ?, ?)',
        [deviceId, ssid, password, new Date().toISOString()],
        (err) => {
            if (err) {
                console.error('Failed to save WiFi credentials:', err);
                return res.status(500).send('Failed to save WiFi credentials');
            }
            res.status(201).send('WiFi credentials saved successfully');
        }
    );
});

app.get('/devices/:deviceId/mac', authenticate, (req, res) => {
    const { deviceId } = req.params;

    db.get(
        'SELECT macAddress FROM IoTDevices WHERE id = ? AND userId = ?',
        [deviceId, req.user.id],
        (err, row) => {
            if (err) {
                console.error('Failed to fetch MAC address:', err);
                return res.status(500).send('Failed to fetch MAC address');
            }
            if (!row) {
                return res.status(404).send('Device not found');
            }
            res.status(200).send({ macAddress: row.macAddress });
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

app.post('/publish-mqtt', authenticate, (req, res) => {
    const { topic, message } = req.body;

    if (!topic || !message) {
        return res.status(400).send('Topic and message are required');
    }

    mqttClient.publish(topic, message, (err) => {
        if (err) {
            console.error('Failed to publish message:', err);
            return res.status(500).send('Failed to publish message');
        }

        console.log(`Message published to topic ${topic}:`, message);
        res.status(200).send(`Message published to topic ${topic}`);
    });
});
