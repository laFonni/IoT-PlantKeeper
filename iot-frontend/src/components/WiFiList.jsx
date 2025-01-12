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

    // Delete a WiFi network
    const deleteWiFi = async (id) => {
        try {
            const response = await axios.delete(`http://localhost:4000/wifi/${id}`, {
                headers: { Authorization: `Bearer ${token}` },
            });
            setNetworks((prevNetworks) => prevNetworks.filter((network) => network.id !== id));
            console.log('WiFi network deleted:', id);
        } catch (err) {
            console.error('Failed to delete WiFi network:', err);
        }
    };
    

    return (
        <div className="mt-6">
            <h2 className="text-xl font-bold mb-4">Your WiFi Networks</h2>
            <ul className="space-y-2">
                {networks.map((network) => (
                    <li key={network.id} className="p-4 bg-gray-100 rounded-md shadow flex justify-between items-center">
                        <p><strong>SSID:</strong> {network.ssid}</p>
                        <button
                            onClick={() => deleteWiFi(network.id)}
                            className="bg-red-500 text-white py-1 px-3 rounded hover:bg-red-600 transition"
                        >
                            Delete
                        </button>
                    </li>
                ))}
            </ul>
        </div>
    );
};

export default WiFiList;
