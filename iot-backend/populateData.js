const sqlite3 = require('sqlite3').verbose();
const bcrypt = require('bcrypt');

const db = new sqlite3.Database('./iot.db', (err) => {
  if (err) {
    console.error('Failed to connect to SQLite database:', err);
  } else {
    console.log('Connected to SQLite database.');
    populateDatabase();
  }
});

async function populateDatabase() {
  const hashedPassword = await bcrypt.hash('1234qwer', 10);
  const userId = await new Promise((resolve, reject) => {
    db.run(
      'INSERT INTO Users (email, password) VALUES (?, ?)',
      ['john.doe@example.com', hashedPassword],
      function (err) {
        if (err) {
          console.error('Failed to insert user:', err);
          reject(err);
        } else {
          resolve(this.lastID);
        }
      }
    );
  });

  const wifiId = await new Promise((resolve, reject) => {
    db.run(
      'INSERT INTO WiFiNetworks (ssid, password, userId) VALUES (?, ?, ?)',
      ['Galaxy A15 57B3', 'wifiPassword', userId],
      function (err) {
        if (err) {
          console.error('Failed to insert WiFi network:', err);
          reject(err);
        } else {
          resolve(this.lastID);
        }
      }
    );
  });

  const deviceId = await new Promise((resolve, reject) => {
    db.run(
      'INSERT INTO IoTDevices (name, macAddress, wifiId, userId) VALUES (?, ?, ?, ?)',
      ['IoT Device 1', '00:1B:44:11:3A:B7', wifiId, userId],
      function (err) {
        if (err) {
          console.error('Failed to insert IoT device:', err);
          reject(err);
        } else {
          resolve(this.lastID);
        }
      }
    );
  });

  const sensorTypes = ['soilMoisture', 'temperature', 'lightLevel'];
  const now = new Date();

  db.serialize(() => {
    db.run('BEGIN TRANSACTION');
    for (let i = 0; i < 24; i++) {
      const timestamp = new Date(now - i * 3600000).toISOString(); // Last 24 hours
      sensorTypes.forEach((type) => {
        const value = type === 'temperature' ? 20 + Math.random() * 10 : Math.random() * 100;
        db.run(
          'INSERT INTO TelemetryData (deviceId, timestamp, sensorType, value) VALUES (?, ?, ?, ?)',
          [deviceId, timestamp, type, value],
          (err) => {
            if (err) {
              console.error('Failed to insert telemetry data:', err);
            }
          }
        );
      });
    }
    db.run('COMMIT');
  });
}
