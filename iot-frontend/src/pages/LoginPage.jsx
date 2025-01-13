import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import LoginForm from '../components/LoginForm';

const LoginPage = () => {
    const [token, setToken] = useState('');
    const navigate = useNavigate();

    const handleLogin = (newToken) => {
        setToken(newToken);
        localStorage.setItem('token', newToken); // Zapisanie tokena w localStorage
        navigate('/dashboard'); // Przekierowanie na dashboard
    };

    return <LoginForm onLogin={handleLogin} />;
};

export default LoginPage;
