from flask import Flask, request, jsonify, send_from_directory, send_file
from flask_cors import CORS, cross_origin
import os
import random
import string
from datetime import datetime

# Configuration
OUTPUT_DIR = "code"
LOG_FILE = "spb.log"
SYMBOLS = string.ascii_lowercase + string.ascii_uppercase + string.digits
ID_LEN = 4
PORT = 9999 
URL = f"https://bin.night0721.xyz"

app = Flask(__name__)
cors = CORS(app)
app.config['CORS_HEADERS'] = 'Content-Type'

os.makedirs(OUTPUT_DIR, exist_ok=True)

def generate_id():
	return ''.join(random.choices(SYMBOLS, k=ID_LEN))

def log_request(ip, hostname, unique_id):
	"""Log the request details."""
	timestamp = datetime.now().ctime()
	with open(LOG_FILE, 'a') as log_file:
		fmtstr = f"[{timestamp}] [{ip}] [{hostname}] [{unique_id}]"
		print(fmtstr)
		log_file.write(fmtstr + "\n")

@app.route('/', methods=['POST'])
@cross_origin()
def upload():
	try:
		id = generate_id()
		if 'file' in request.files:
			file = request.files['file']
			if file.filename == '':
				return jsonify({"error": "No file selected"}), 400
			filename = id
			file.save(os.path.join(OUTPUT_DIR, filename))
		else:
			data = request.data.decode('utf-8')
			if not data:
				return jsonify({"error": "No data provided"}), 400
			filename = os.path.join(OUTPUT_DIR, id)
			with open(filename, 'w') as f:
				f.write(data)
				f.write("\n")
				f.close()
		#		    IP					 Hostname
		log_request(request.remote_addr, request.host, filename)

		return f"{URL}/{id}\n"
	except Exception as e:
		return jsonify({"error": str(e)}), 500

@app.route('/', methods=['GET'])
@cross_origin()
def index():
	return send_from_directory('.', 'index.html')

@app.route("/<id>", methods=["GET"])
def get_file(id):
    file_path = os.path.join(OUTPUT_DIR, id)

    if not os.path.isfile(file_path):
        return jsonify({"error": f"File '{id}' not found"}), 404

    try:
        # Send the file content
        return send_file(file_path, as_attachment=False)
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == "__main__":
	app.run(host="0.0.0.0", port=PORT, debug=False)
