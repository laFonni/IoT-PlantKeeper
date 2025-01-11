import React, { useState, useEffect, useContext } from 'react';
import AddWiFiForm from '../components/AddWiFiForm';
import WiFiList from '../components/WiFiList';
import { AuthContext } from '../AuthContext';
import axios from 'axios';

const WiFiPage = () => {
    const { token } = useContext(AuthContext); // Access token from AuthContext
    const [networks, setNetworks] = useState([]);

    const fetchWiFiNetworks = async () => {
        try {
            const response = await axios.get('http://localhost:4000/wifi', {
                headers: { Authorization: `Bearer ${token}` },
            });
            setNetworks(response.data);
        } catch (err) {
            console.error('Failed to fetch WiFi networks:', err);
        }
    };

    const handleAddWiFi = (newNetwork) => {
        console.log('Adding new network to state:', newNetwork);
        setNetworks((prevNetworks) => [...prevNetworks, newNetwork]); // Update networks state
    };

    useEffect(() => {
        fetchWiFiNetworks();
    }, [token]);

    return (
        <div className="p-6">
            <AddWiFiForm onAddWiFi={handleAddWiFi} /> {/* Pass handleAddWiFi */}
            <WiFiList networks={networks} />
        </div>
    );
};

export default WiFiPage;
