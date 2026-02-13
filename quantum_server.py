import http.server
import socketserver
import json
import logging
from datetime import datetime
from qiskit import QuantumCircuit
from qiskit_aer import AerSimulator

# --- CONFIGURACIÓN ---
PORT = 8609
IP = "91.99.90.39"
LOG_FILE = "servidor_logs.txt"

# SIMULADOR CUÁNTICO GLOBAL
QUANTUM_SIMULATOR = AerSimulator()

# Configuración de Logging
logging.basicConfig(
    filename=LOG_FILE,
    level=logging.INFO,
    format="%(asctime)s | %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)

class ThreadedReusableServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    allow_reuse_address = True
    daemon_threads = True

class QuantumHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/generate_bit":
            try:
                # Crear circuito cuántico (1 Qubit, 1 Bit Clásico)
                circuit = QuantumCircuit(1, 1)

                # Puerta Hadamard (Superposición 50/50)
                circuit.h(0)

                # Medir el colapso
                circuit.measure(0, 0)

                # Ejecutar simulación
                result = QUANTUM_SIMULATOR.run(circuit, shots=1, memory=True).result()
                memory = result.get_memory(circuit)
                quantum_bit = int(memory[0]) # Resultado: 0 o 1

                # Preparar respuesta
                response = {
                    "success": True,
                    "value": quantum_bit,
                    "source": "quantum_simulation",
                    "timestamp": datetime.now().strftime("%H:%M:%S")
                }

                # Enviar cabeceras
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.send_header("Access-Control-Allow-Origin", "*")
                self.end_headers()

                # Enviar cuerpo
                self.wfile.write(json.dumps(response).encode("utf-8"))

                logging.info(f"Petición Cuántica desde {self.client_address[0]} | Resultado: {quantum_bit}")
                return

            except Exception as e:
                logging.error(f"Error Cuántico: {e}")
                self.send_error(500, str(e))
                return
        
        # Respuesta por defecto para raíz o rutas desconocidas
        self.send_response(200)
        self.send_header("Content-type", "text/plain; charset=utf-8")
        self.end_headers()
        self.wfile.write(b"Servidor Cuántico Ejecutándose. Usa /generate_bit para obtener un bit aleatorio.")

    def log_message(self, format, *args):
        # Silenciar logging por defecto a consola, confiar en la configuración custom
        pass

if __name__ == "__main__":
    print(f"Servidor Cuántico corriendo en el puerto {PORT}...")
    print(f"IP Configurada: {IP}") 
    print(f"Logs en {LOG_FILE}")
    
    # Se utiliza 0.0.0.0 para escuchar en todas las interfaces, usando el puerto definido
    with ThreadedReusableServer(("0.0.0.0", PORT), QuantumHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass
