**oZmlink** es el gestor encargado de configurar la placa Orange Pi Zero como un punto de acceso para que un usuario pueda, a su vez, proporcionar a **oZm** las credenciales de acceso y conexión como cliente y disponer de acceso a Internet.

**oZmlink** hace uso de las siguientes herramientas para lograr su cometido:
* Servidor web Apache + PHP
* Dnsmasq
* Hostapd
* Wpa_supplicant

# Instalación
1. Copiamos el tar **ozmlink-install.tar.gz** y los descomprimimos mediante **tar xzf ozmlink.tar.gz**
2. Ejecutamos el script de instalación **install.sh**
3. Con esto ya tendremos todo lo necesario para arrancar la placa en modo AP, conectarnos a la misma como cliente Wifi y configurar el cambio a modo STA especificando las credenciales necesarias.

Todo lo necesario para **oZmlink** se ubica bajo la ruta **/usr/local/ozmlink**.

**Nota:** La instalación de **oZmlink** deshabilita el arranque automático de los servicios **network-manager**, **dnsmasq** y **hostapd**.

# Cambio a modo AP
1. Ejecutaremos el comando **/usr/local/ozmlink/bin/setmodoap.sh**
2. Reiniciaremos la placa

Una vez arranque, se creará una red con SSID **OZM-CONFIG** a la que deberemos conectarnos para proceder a establecer la configuración para el modo STA. En cuanto nos conectemos, se nos abrirá un asistente que nos guiará para la selección de la red y las credenciales de acceso a la misma. Una vez configurada la contraseña, se procederá a reiniciar la placa en modo STA. Si pasados 60 segundos desde el reinicio no se obtiene acceso a Internet, el sistema se volverá a reiniciar en modo AP.

**Nota:** En caso de que el asistente no se abra de forma automática, usaremos nuestro navegador para ir hacia http://ozm.config

# Cambio a modo STA
Si por algún motivo deseamos cambiar a modo STA desde la línea de comandos, ejecutaremos **/usr/local/ozmlink/bin/setmodosta.sh SSID PASSWORD**, siendo **SSID** el ssid de la red a la que deseamos conectar y **PASSWORD** la contraseña de la misma. La ejecución de este comando produce un reinicio automático.

# Logging
En la ruta **/usr/local/ozmlink/log/ozmlink.log** tenemos un archivo de log con alguna información útil por si nos fuera necesario.
