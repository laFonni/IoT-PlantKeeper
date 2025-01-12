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

  // Insert first user
  const userId1 = await new Promise((resolve, reject) => {
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

  // Insert second user
  const userId2 = await new Promise((resolve, reject) => {
    db.run(
      'INSERT INTO Users (email, password) VALUES (?, ?)',
      ['jane.doe@example.com', hashedPassword],
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

  // Insert WiFi networks for both users
  const wifiId1 = await new Promise((resolve, reject) => {
    db.run(
      'INSERT INTO WiFiNetworks (ssid, password, userId) VALUES (?, ?, ?)',
      ['Galaxy A15 57B3', 'wifiPassword', userId1],
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

  const wifiId2 = await new Promise((resolve, reject) => {
    db.run(
      'INSERT INTO WiFiNetworks (ssid, password, userId) VALUES (?, ?, ?)',
      ['Galaxy A15 57B4', 'wifiPassword', userId2],
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

  // Insert devices for both users
  const deviceId1 = await new Promise((resolve, reject) => {
    db.run(
      'INSERT INTO IoTDevices (name, macAddress, wifiId, userId) VALUES (?, ?, ?, ?)',
      ['IoT Device 1', '00:1B:44:11:3A:B7', wifiId1, userId1],
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

  const deviceId2 = await new Promise((resolve, reject) => {
    db.run(
      'INSERT INTO IoTDevices (name, macAddress, wifiId, userId) VALUES (?, ?, ?, ?)',
      ['IoT Device 2', '00:1B:44:11:3A:B8', wifiId2, userId2],
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

  // Insert telemetry data for both devices
  const sensorTypes = ['soilMoisture', 'temperature', 'lightLevel'];
  const actuatorTypes = ['waterPump', 'lamp'];
  const actuatorStates = [0, 1]; // 0 for off, 1 for on
  const now = new Date();

  db.serialize(() => {
    db.run('BEGIN TRANSACTION');
    for (let i = 0; i < 24; i++) {
      const timestamp = new Date(now - i * 3600000).toISOString(); // Last 24 hours
      sensorTypes.forEach((type) => {
        const value = type === 'temperature' ? 20 + Math.random() * 10 : Math.random() * 100;
        db.run(
          'INSERT INTO TelemetryData (deviceId, timestamp, sensorType, value) VALUES (?, ?, ?, ?)',
          [deviceId1, timestamp, type, value],
          (err) => {
            if (err) {
              console.error('Failed to insert telemetry data:', err);
            }
          }
        );
        db.run(
          'INSERT INTO TelemetryData (deviceId, timestamp, sensorType, value) VALUES (?, ?, ?, ?)',
          [deviceId2, timestamp, type, value],
          (err) => {
            if (err) {
              console.error('Failed to insert telemetry data:', err);
            }
          }
        );
      });

      actuatorTypes.forEach((type) => {
        const value = actuatorStates[Math.floor(Math.random() * actuatorStates.length)];
        db.run(
          'INSERT INTO TelemetryData (deviceId, timestamp, sensorType, value) VALUES (?, ?, ?, ?)',
          [deviceId1, timestamp, type, value],
          (err) => {
            if (err) {
              console.error('Failed to insert actuator data:', err);
            }
          }
        );
        db.run(
          'INSERT INTO TelemetryData (deviceId, timestamp, sensorType, value) VALUES (?, ?, ?, ?)',
          [deviceId2, timestamp, type, value],
          (err) => {
            if (err) {
              console.error('Failed to insert actuator data:', err);
            }
          }
        );
      });
    }
    db.run('COMMIT');
  });
}
