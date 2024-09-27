from flask import Flask, request, jsonify, render_template_string

app = Flask(__name__)

# A simple in-memory storage
data_store = {"location": "Santa Cruz",
                "outside": "N/A",
                "ambient": "N/A",
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
    <p>Location: <span id="location">{{ location }}</span></p>
    <p>Outside Temperature: <span id="outside">{{ outside }}</span></p>
    <p>Ambient Temperature: <span id="ambient">{{ ambient }}</span>°F</p>
    <p>Humidity: <span id="humidity">{{ humidity }}</span>%RH</p>
    <button onclick="updateValue()">Refresh</button>
    <script>
        function updateValue() {
            fetch('/get_data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('outside').textContent = data.outside;
                    document.getElementById('ambient').textContent = data.ambient;
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
    return render_template_string(html_template, location=data_store["location"], outside=data_store["outside"], ambient=data_store["ambient"], humidity=data_store["humidity"])

@app.route('/post', methods=['POST'])
def post_data():
    data = request.get_json()
    if data and "outside" in data and "ambient" in data and "humidity" in data:
        data_store["outside"] = data["outside"]
        data_store["ambient"] = data["ambient"]
        data_store["humidity"] = data["humidity"]
        return jsonify({
            'status': 'success',
            'outside': data_store["outside"],
            'ambient': data_store["ambient"],
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
    return jsonify({"outside": data_store["outside"], "ambient": data_store["ambient"], "humidity": data_store["humidity"]})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=1234)

