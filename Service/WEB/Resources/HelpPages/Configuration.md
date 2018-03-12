The openZmeter service reads the configuration file `/etc/openZmeter.conf` at startup. This file consists of sections that contain options of the form `param = value`.  
It is possible to run several openZmeter services on the same computer, for this you can use the option `--config=rutaarchivo`.  

The available options are:

### [global]
- `listen = ruta` : Path to the local socket through which the service can be interrogated.
- `log = ruta` : Path to the log file, where the generated messages will be saved.
- `pid = ruta` : Path to the PID file, which may use the page to restart the service and reload the configuration.

### [database]
- `host = ip` : ip IP address or host name where postgre is executed.
- `port = 5432` : 5432 Server port (usually 5432).
- `database = openzmeter` : openzmeter Name of the database to be used.
- `user = openzmeter` : openzmeter Username.
- `pass = OyhzA7rU` : OyhzA7rU Password for the user.

### [input]
 - `module = 'type' ` : Where type is the module to use. Currently it can be 'BasicCapture.ko'.
electrical

    ### electrical
- `nominal_frequency = 50` : Indicates the nominal frequency of the network, depending on this, the analysis and storage algorithms will be adjusted. This value will be used for the internal analyzer and will be sent to the distributed nodes during their connection to apply it.
- `voltage_gain = xxx` : Indicates the gain to be applied in the voltage channel to the directly connected node.
- `voltage_offset = xxx` : Indicates the offset to be applied in the voltage channel to the directly connected node.
- `current_gain = xxx` : Indicates the gain to be applied in the current channel to the directly connected node.
- `gurrent_offset = xxx` : Indicates the offset to be applied in the current channel to the directly connected node.  

### node
This group must be included for each distributed node in the network.
- `ip = xxx.xxx.xxx.xxx` : Indicates the node’s IP address.
- `id = mmmmm` : Indicates a unique number assigned to the node.
- `voltage_gain = xxx` : : Indicates the gain to apply in the voltage channel.
- `voltage_offset = xxx` : Indicates the offset to apply in the voltage channel.
- `current_gain = xxx` : Indicates the gain to apply in the current channel.
- `gurrent_offset = xxx` : Indicates the offset to apply in the current channel.
