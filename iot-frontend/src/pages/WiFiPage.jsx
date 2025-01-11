import React, { useContext, useState } from 'react';
import AddWiFiForm from '../components/AddWiFiForm';
import WiFiList from '../components/WiFiList';
import { AuthContext } from '../AuthContext';

const WiFiPage = () => {
    const { token } = useContext(AuthContext);
    console.log('Token in WiFiPage:', token);

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

    useEffect(() => {
        fetchWiFiNetworks();
    }, [token]);

    const handleAddWiFi = (newNetwork) => {
        setNetworks((prevNetworks) => [...prevNetworks, newNetwork]);
    };

    return (
        <div className="p-6">
            <AddWiFiForm token={token} onAddWiFi={handleAddWiFi} />
            <WiFiList token={token} />
        </div>
    );
};

export default WiFiPage;
