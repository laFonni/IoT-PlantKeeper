import React, { useState, useContext } from 'react';
import axios from 'axios';
import { AuthContext } from '../AuthContext';

const AddDeviceForm = ({ onAddDevice }) => {
  const { token } = useContext(AuthContext);
  const [name, setName] = useState('');
  const [macAddress, setMacAddress] = useState('');
  const [wifiId, setWifiId] = useState('');
  const [status, setStatus] = useState('');

  const handleAddDevice = async (e) => {
    e.preventDefault();

    try {
      const response = await axios.post(
        'http://localhost:4000/devices',
        { name, macAddress, wifiId },
        { headers: { Authorization: `Bearer ${token}` } }
      );
      setStatus('Device added successfully!');
      setName('');
      setMacAddress('');
      setWifiId('');
      if (onAddDevice) {
        onAddDevice(response.data); // Call parent handler with new device
      }
    } catch (err) {
      console.error('Failed to add device:', err);
      setStatus('Failed to add device.');
    }
  };

  return (
    <form onSubmit={handleAddDevice} className="flex flex-col gap-4 bg-white p-6 rounded-lg shadow-md">
      <h2 className="text-xl font-bold mb-4">Add Device</h2>
      <input
        type="text"
        placeholder="Device Name"
        value={name}
        onChange={(e) => setName(e.target.value)}
        required
        className="p-2 border rounded-md"
      />
      <input
        type="text"
        placeholder="MAC Address"
        value={macAddress}
        onChange={(e) => setMacAddress(e.target.value)}
        required
        className="p-2 border rounded-md"
      />
      <input
        type="text"
        placeholder="WiFi ID"
        value={wifiId}
        onChange={(e) => setWifiId(e.target.value)}
        required
        className="p-2 border rounded-md"
      />
      <button
        type="submit"
        className="bg-blue-500 text-white py-2 px-4 rounded-md hover:bg-blue-600 transition"
      >
        Add Device
      </button>
      {status && (
        <p className={`text-center mt-4 ${status.includes('successfully') ? 'text-green-500' : 'text-red-500'}`}>
          {status}
        </p>
      )}
    </form>
  );
};

export default AddDeviceForm;
