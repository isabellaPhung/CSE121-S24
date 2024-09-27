from flask import Flask, request, jsonify, render_template_string

app = Flask(__name__)

# A simple in-memory storage
data_store = {"location": "Santa Cruz",
                "temperature": "N/A",
                "humidity": "N/A"}

# HTML template with JavaScript to update the data
html_template = """
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Data Viewer</title>
</head>
<body>
    <h1>ESP32 Data Viewer</h1>
    <p>Ambient Temperature: <span id="temperature">{{ temperature }}</span>°F</p>
    <p>Humidity: <span id="humidity">{{ humidity }}</span>%RH</p>
    <button onclick="updateValue()">Refresh</button>
    <script>
        function updateValue() {
            fetch('/get_data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('temperature').textContent = data.temperature;
                    document.getElementById('humidity').textContent = data.humidity;
                });
        }

        // Automatically update every 1 seconds
        setInterval(updateValue, 1000);
    </script>
</body>
</html>
"""

@app.route('/')
def index():
    return render_template_string(html_template, temperature=data_store["temperature"], humidity=data_store["humidity"])

@app.route('/post', methods=['POST'])
def post_data():
    data = request.get_json()
    if "temperature" in data and "humidity" in data:
        data_store["temperature"] = data["temperature"]
        data_store["humidity"] = data["humidity"]
        return jsonify({
            'status': 'success',
            'temperature': data_store["temperature"],
            'humidity': data_store["humidity"]
        }), 200
    else:
        return jsonify({
            'status': 'fail',
            'message': 'Invalid data received'
        }), 400

@app.route('/location', methods=['GET'])
def location():
    return jsonify({"location": data_store["location"]})

@app.route('/get_data', methods=['GET'])
def get_data():
    return jsonify({"temperature": data_store["temperature"], "humidity": data_store["humidity"]})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=1234)

