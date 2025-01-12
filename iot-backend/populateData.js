const sqlite3 = require('sqlite3').verbose();

const db = new sqlite3.Database('./iot.db', (err) => {
  if (err) {
    console.error('Failed to connect to SQLite database:', err);
  } else {
    console.log('Connected to SQLite database.');
    populateDatabase();
  }
});

function populateDatabase() {
  const deviceId = 1; // Example device ID
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