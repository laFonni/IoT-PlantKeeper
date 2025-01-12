import React, { useState, useContext } from 'react';
import axios from 'axios';
import { AuthContext } from '../AuthContext';

const AddWiFiForm = ({ onAddWiFi }) => {
    const { token } = useContext(AuthContext); // Access token from AuthContext
    const [ssid, setSsid] = useState('');
    const [password, setPassword] = useState('');
    const [status, setStatus] = useState('');

    const handleAddWiFi = async (e) => {
        e.preventDefault();

        try {
            const response = await axios.post(
                'http://localhost:4000/wifi',
                { ssid, password },
                { headers: { Authorization: `Bearer ${token}` } }
            );
            console.log('Backend response:', response.data);
            setStatus('WiFi added successfully!');
            setSsid('');
            setPassword('');
            if (onAddWiFi) {
                onAddWiFi(response.data); // Call parent handler with new network
            }
        } catch (err) {
            console.error('Failed to add WiFi:', err);
            setStatus('Failed to add WiFi.');
        }
    };

    return (
        <form onSubmit={handleAddWiFi} className="flex flex-col gap-4 bg-white p-6 rounded-lg shadow-md">
            <h2 className="text-xl font-bold mb-4">Add WiFi Network</h2>
            <input
                type="text"
                placeholder="SSID"
                value={ssid}
                onChange={(e) => setSsid(e.target.value)}
                required
                className="p-2 border rounded-md"
            />
            <input
                type="password"
                placeholder="Password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                required
                className="p-2 border rounded-md"
            />
            <button
                type="submit"
                className="bg-blue-500 text-white py-2 px-4 rounded-md hover:bg-blue-600 transition"
            >
                Add WiFi
            </button>
            {status && (
                <p className={`text-center mt-4 ${status.includes('successfully') ? 'text-green-500' : 'text-red-500'}`}>
                    {status}
                </p>
            )}
        </form>
    );
};

export default AddWiFiForm;
