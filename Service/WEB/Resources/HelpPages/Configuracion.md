El servicio openZmeter lee la configuración del archivo `/etc/openZmeter.conf` al iniciarse. Este archivo esta formado por secciones `[section]` que contienen opciones de configuración de la forma `param = value`.

Es posible ejecutar varios servicios openZmeter en un mismo ordenador, para esto se puede usar la opción `--config=rutaarchivo`.

Las opciones disponibles son:

### [global]
- `listen = ruta` : Ruta al socket local por el que podrá interrogarse al servicio.
- `log = ruta` : Ruta al archivo de registro, donde se guardaran los mensajes generados.
- `pid = ruta` : Ruta al archivo PID, que podrá usar la pagina para reiniciar el servicio y recargar la configuración.

### [database]
- `host = ip` : Direccion IP o nombre de host donde se ejecuta postgre.
- `port = 5432` : Puerto del servidor (habitualmente 5432).
- `database = openzmeter` : Nombre de la base de datos a usar.
- `user = openzmeter` : Nombre de usuario.
- `pass = OyhzA7rU` : Contraseña para el usuario.

### [input]
 - `module = 'type' ` : Donde type es el modulo a usar.  Actualmente puede ser 'BasicCapture.ko'.
### electrical
- `nominal_frequency = 50` : Indica la frecuencia nominal de red, en función de esta se ajustaran los algoritmos de análisis y almacenamiento. Este valor se utilizara para el analizador interno y sera enviado a los nodos distribuidos durante su conexion para que lo apliquen.
- `voltage_gain = xxx` : Indica la ganancia a aplicar en el canal de tension al nodo conectado directamente.
- `voltage_offset = xxx` : Indica el offset a aplicar en el canal de tension al nodo conectado directamente.
- `current_gain = xxx` : Indica la ganancia a aplicar en el canal de corriente al nodo conectado directamente.
- `gurrent_offset = xxx` : Indica el offset a aplicar en el canal de corriente al nodo conectado directamente.

### node
Este grupo debe incluirse para cada nodo distribuido existente en la red.
- `ip = xxx.xxx.xxx.xxx` : Indica la dirección IP del nodo.
- `id = mmmmm` : Indica un numero único asignado al nodo.
- `voltage_gain = xxx` : Indica la ganancia a aplicar en el canal de tensión.
- `voltage_offset = xxx` : Indica el offset a aplicar en el canal de tensión.
- `current_gain = xxx` : Indica la ganancia a aplicar en el canal de corriente.
- `gurrent_offset = xxx` : Indica el offset a aplicar en el canal de corriente.
