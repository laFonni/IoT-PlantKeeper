import React, { useEffect, useState } from 'react';
import axios from 'axios';

const WiFiList = ({ token }) => {
    const [networks, setNetworks] = useState([]);

    useEffect(() => {
        const fetchWiFiNetworks = async () => {
            console.log('Token in WiFiList:', token);

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
