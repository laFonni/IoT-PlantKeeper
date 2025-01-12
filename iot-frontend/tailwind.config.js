/** @type {import('tailwindcss').Config} */
module.exports = {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        primary: '#2E7D32', // Darker Green
        secondary: '#7CB342', // Light Green
        accent: '#FDD835', // Warm Yellow
        background: '#F1F8E9', // Light Green Background
        leaf: '#81C784', // Leaf Green
        soil: '#795548', // Brown
        sky: '#90CAF9', // Light Blue
      },
      fontFamily: {
        sans: ['"Open Sans"', 'sans-serif'],
        serif: ['"Merriweather"', 'serif'],
      },
      backgroundImage: {
        'leaf-pattern': "url('data:image/svg+xml,...')" // You would need to add an actual SVG pattern here
      }
    },
  },
  plugins: [],
};
