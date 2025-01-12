import React, { createContext, useState, useEffect } from 'react';

export const AuthContext = createContext();

const AuthProvider = ({ children }) => {
    const [token, setToken] = useState(null);
    const [email, setEmail] = useState(null);

    useEffect(() => {
        const savedToken = localStorage.getItem('token');
        const savedEmail = localStorage.getItem('email');
        if (savedToken) {
            setToken(savedToken);
        }
        if (savedEmail) {
            setEmail(savedEmail);
        }
    }, []);

    const login = (newToken, userEmail) => {
        setToken(newToken);
        setEmail(userEmail); // Store the email
        localStorage.setItem('token', newToken);
        localStorage.setItem('email', userEmail); // Save email to localStorage
    };

    const logout = () => {
        console.log('Clearing token from AuthContext');
        setToken(null);
        localStorage.removeItem('token');
    };

    return (
        <AuthContext.Provider value={{ token, email, login, logout }}>
            {children}
        </AuthContext.Provider>
    );
};

export default AuthProvider;
