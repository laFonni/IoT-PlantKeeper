import React, { useState } from 'react';
import axios from 'axios';
import {useNavigate} from 'react-router-dom';

const RegisterForm = () => {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [status, setStatus] = useState('');
  const navigate = useNavigate();

  const handleRegister = async (e) => {
    e.preventDefault();
  
    if (password !== confirmPassword) {
      setStatus('Passwords do not match.');
      return;
    }
  
    console.log('Sending request to backend:', { email, password }); // Logowanie danych
  
    try {
      const response = await axios.post('http://localhost:4000/register', { email, password });
      console.log('Response from backend:', response.data); // Logowanie odpowiedzi
      setStatus('Registration successful!');
      setTimeout(() => {
        navigate('/login');
      }, 1500);
    } catch (err) {
      console.error('Error from backend:', err); // Logowanie błędów
      setStatus('Registration failed. User might already exist.');
    }
  };
  

  return (
    <form
      onSubmit={handleRegister}
      className="flex flex-col gap-4 p-6 max-w-md mx-auto mt-10 bg-white rounded-lg shadow-md border"
    >
      <h2 className="text-2xl font-bold text-center text-gray-700">Register</h2>
      <input
        type="email"
        placeholder="Email"
        value={email}
        onChange={(e) => setEmail(e.target.value)}
        required
        className="p-2 border rounded-md focus:outline-none focus:ring focus:ring-blue-300"
      />
      <input
        type="password"
        placeholder="Password"
        value={password}
        onChange={(e) => setPassword(e.target.value)}
        required
        className="p-2 border rounded-md focus:outline-none focus:ring focus:ring-blue-300"
      />
      <input
        type="password"
        placeholder="Confirm Password"
        value={confirmPassword}
        onChange={(e) => setConfirmPassword(e.target.value)}
        required
        className="p-2 border rounded-md focus:outline-none focus:ring focus:ring-blue-300"
      />
      <button
        type="submit"
        className="bg-blue-500 text-white font-semibold py-2 px-4 rounded-md hover:bg-blue-600 transition"
      >
        Register
      </button>
      {status && (
        <p className={`text-center mt-4 ${status.includes('successful') ? 'text-green-500' : 'text-red-500'}`}>
          {status}
        </p>
      )}
    </form>
  );
};

export default RegisterForm;
