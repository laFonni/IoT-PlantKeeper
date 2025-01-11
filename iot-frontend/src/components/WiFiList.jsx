import React, { useEffect, useState, useContext } from 'react';
import axios from 'axios';
import { AuthContext } from '../AuthContext';

const WiFiList = () => {
    const { token } = useContext(AuthContext); // Access token from AuthContext
    const [networks, setNetworks] = useState([]);

    useEffect(() => {
        const fetchWiFiNetworks = async () => {

            if (!token) {
                console.error('No token available in WiFiList');
                return;
            }

            try {
                const response = await axios.get('http://localhost:4000/wifi', {
                    headers: { Authorization: `Bearer ${token}` },
                });
                setNetworks(response.data);
            } catch (err) {
                console.error('Failed to fetch WiFi networks:', err);
            }
        };

        fetchWiFiNetworks();
    }, [token]);

    return (
        <div className="mt-6">
            <h2 className="text-xl font-bold mb-4">Your WiFi Networks</h2>
            <ul className="space-y-2">
                {networks.map((network) => (
                    <li key={network.id} className="p-4 bg-gray-100 rounded-md shadow">
                        <p><strong>SSID:</strong> {network.ssid}</p>
                    </li>
                ))}
            </ul>
        </div>
    );
};

export default WiFiList;
