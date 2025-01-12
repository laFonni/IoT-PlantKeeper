import React, { useEffect, useState, useContext } from 'react';
import axios from 'axios';
import { useNavigate } from 'react-router-dom';
import { AuthContext } from '../AuthContext';

const DeviceList = () => {
  const { token } = useContext(AuthContext);
  const navigate = useNavigate();
  const [deviceList, setDeviceList] = useState([]);

  useEffect(() => {
    const fetchDevices = async () => {
      if (!token) {
        console.error('No token available in DeviceList');
        return;
      }

      try {
        const response = await axios.get('http://localhost:4000/devices', {
          headers: { Authorization: `Bearer ${token}` },
        });
        setDeviceList(response.data);
      } catch (err) {
        console.error('Failed to fetch devices:', err);
      }
    };
    fetchDevices();
  }, [token]);

  return (
    <div className="p-6 max-w-3xl mx-auto">
      <h2 className="text-2xl font-bold mb-4 text-gray-700">Your Devices</h2>
      <ul className="space-y-4">
        {deviceList.map((device) => (
          <li
            key={device.id}
            className="p-4 border rounded-lg shadow-md flex justify-between items-center bg-gray-50 hover:bg-gray-100 transition"
          >
            <div>
              <p className="font-bold text-gray-800">{device.name}</p>
              <p className="text-sm text-gray-500">MAC: {device.macAddress}</p>
            </div>
            <button
              onClick={() => navigate(`/device/${device.id}`)}
              className="bg-blue-500 text-white py-1 px-3 rounded-md hover:bg-blue-600 transition"
            >
              View Details
            </button>
          </li>
        ))}
      </ul>
    </div>
  );
};

export default DeviceList;
