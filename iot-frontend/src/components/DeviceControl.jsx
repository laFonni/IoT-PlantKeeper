import React, { useState } from 'react';
import axios from 'axios';

const DeviceControl = ({ token, deviceId }) => {
  const [status, setStatus] = useState('');

  const sendCommand = async (action) => {
    try {
      await axios.post(
        `http://localhost:4000/devices/${deviceId}/command`,
        { action, device: 'pump' },
        { headers: { Authorization: `Bearer ${token}` } }
      );
      setStatus(`Command "${action}" sent successfully.`);
    } catch (err) {
      alert('Failed to send command');
    }
  };

  return (
    <div className="p-6 max-w-lg mx-auto mt-10 bg-white rounded-lg shadow-md border">
      <h2 className="text-2xl font-bold mb-4 text-gray-700">Control Device {deviceId}</h2>
      <div className="flex gap-4">
        <button
          onClick={() => sendCommand('turn_on')}
          className="bg-green-500 text-white py-2 px-4 rounded-md hover:bg-green-600 transition"
        >
          Turn On Pump
        </button>
        <button
          onClick={() => sendCommand('turn_off')}
          className="bg-red-500 text-white py-2 px-4 rounded-md hover:bg-red-600 transition"
        >
          Turn Off Pump
        </button>
      </div>
      <p className="mt-4 text-gray-600">{status}</p>
    </div>
  );
};

export default DeviceControl;
