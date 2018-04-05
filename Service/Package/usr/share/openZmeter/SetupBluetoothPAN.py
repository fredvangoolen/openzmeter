#!/usr/bin/env python
from __future__ import absolute_import, print_function, unicode_literals

from optparse import OptionParser
import sys
import dbus
import dbus.service
import dbus.mainloop.glib
import gobject as GObject
import subprocess
import os

BUS_NAME = "org.bluez"
ADAPTER_INTERFACE = BUS_NAME + ".Adapter1"
AGENT_INTERFACE = BUS_NAME + ".Agent1"
DEVICE_INTERFACE = BUS_NAME + ".Device1"
AGENT_PATH = "/oZm_BT/agent"

bus = None
device_obj = None
dev_path = None
def ask(prompt):
	try:
		return raw_input(prompt)
	except:
		return input(prompt)
def set_trusted(path):
	props = dbus.Interface(bus.get_object(BUS_NAME, path), "org.freedesktop.DBus.Properties")
	props.Set(DEVICE_INTERFACE, "Trusted", True)
def dev_connect(path):
	dev = dbus.Interface(bus.get_object(BUS_NAME, path), DEVICE_INTERFACE)
	dev.Connect()
class Agent(dbus.service.Object):
	exit_on_release = True
	def set_exit_on_release(self, exit_on_release):
		self.exit_on_release = exit_on_release
	@dbus.service.method(AGENT_INTERFACE, in_signature="", out_signature="")
	def Release(self):
		print("Release")
		if self.exit_on_release:
			mainloop.quit()
	@dbus.service.method(AGENT_INTERFACE, in_signature="os", out_signature="")
	def AuthorizeService(self, device, uuid):
		print("New authorized client")
		return
	@dbus.service.method(AGENT_INTERFACE, in_signature="o", out_signature="s")
	def RequestPinCode(self, device):
		set_trusted(device)
		return "1234"
	@dbus.service.method(AGENT_INTERFACE,	in_signature="o", out_signature="u")
	def RequestPasskey(self, device):
		print("RequestPasskey (%s)" % (device))
		set_trusted(device)
		passkey = ask("Enter passkey: ")
		return dbus.UInt32(passkey)
	@dbus.service.method(AGENT_INTERFACE, in_signature="ouq", out_signature="")
	def DisplayPasskey(self, device, passkey, entered):
		print("DisplayPasskey (%s, %06u entered %u)" %
						(device, passkey, entered))
	@dbus.service.method(AGENT_INTERFACE, in_signature="os", out_signature="")
	def DisplayPinCode(self, device, pincode):
		print("DisplayPinCode (%s, %s)" % (device, pincode))
	@dbus.service.method(AGENT_INTERFACE, in_signature="ou", out_signature="")
	def RequestConfirmation(self, device, passkey):
		print("RequestConfirmation (%s, %06d)" % (device, passkey))
		confirm = ask("Confirm passkey (yes/no): ")
		if (confirm == "yes"):
			set_trusted(device)
			return
		raise Rejected("Passkey doesn't match")
	@dbus.service.method(AGENT_INTERFACE, in_signature="o", out_signature="")
	def RequestAuthorization(self, device):
		print("RequestAuthorization (%s)" % (device))
		auth = ask("Authorize? (yes/no): ")
		if (auth == "yes"):
			return
		raise Rejected("Pairing rejected")
	@dbus.service.method(AGENT_INTERFACE, in_signature="", out_signature="")
	def Cancel(self):
		print("Cancel")
def pair_reply():
	print("Device paired")
	set_trusted(dev_path)
	dev_connect(dev_path)
	mainloop.quit()
def pair_error(error):
	err_name = error.get_dbus_name()
	if err_name == "org.freedesktop.DBus.Error.NoReply" and device_obj:
		print("Timed out. Cancelling pairing")
		device_obj.CancelPairing()
	else:
		print("Creating device failed: %s" % (error))
	mainloop.quit()
def main():
	print("Bluetooth PAN server configuration for NanoPI Air (openZmeter)")
	brctl = subprocess.Popen(['brctl', 'show', "br0"], stdout=open(os.devnull, 'wb'), stderr=subprocess.PIPE)
	brctl_stderr = brctl.stderr.read()
	if brctl.wait() or brctl_stderr:
		subprocess.call(['brctl', 'addbr', "br0"])
		subprocess.call(['ifconfig', 'br0', "10.0.0.1"])
		subprocess.call(['ifconfig', 'br0', "up"])
		subprocess.Popen(['dnsmasq', '--interface', 'br0', '--no-daemon', '--dhcp-range', '10.0.0.2,10.0.0.8'], stdout=open(os.devnull, 'wb'))
	else:
		print("Bridge br0 allready exist")
		sys.exit()
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
	bus = dbus.SystemBus()

	adapter_props = dbus.Interface(bus.get_object('org.bluez', '/org/bluez/hci0'), "org.freedesktop.DBus.Properties");
	adapter_props.Set(ADAPTER_INTERFACE, "Powered", dbus.Boolean(1))
	adapter_props.Set(ADAPTER_INTERFACE, "Pairable", dbus.Boolean(1))
	adapter_props.Set(ADAPTER_INTERFACE, "Discoverable", dbus.Boolean(1))
	print("HCI0 set as powered, discoverable and pairable")

	agent = Agent(bus, AGENT_PATH)
	obj = bus.get_object(BUS_NAME, "/org/bluez");
	manager = dbus.Interface(obj, "org.bluez.AgentManager1")
	manager.RegisterAgent(AGENT_PATH, "NoInputNoOutput")
	manager.RequestDefaultAgent(AGENT_PATH)
	print("BT agent registered with fixed PIN 1234")

	server = dbus.Interface(bus.get_object('org.bluez', '/org/bluez/hci0'), "org.bluez.NetworkServer1")
	server.Register("nap", "br0")
	print("Using br0 as network master device")

	mainloop = GObject.MainLoop()
	mainloop.run()

if __name__ == '__main__':
	try:
		main()
	except KeyboardInterrupt:
		bus = dbus.SystemBus()
		adapter_props = dbus.Interface(bus.get_object('org.bluez', '/org/bluez/hci0'), "org.freedesktop.DBus.Properties");
		adapter_props.Set(ADAPTER_INTERFACE, "Powered", dbus.Boolean(0))
		adapter_props.Set(ADAPTER_INTERFACE, "Pairable", dbus.Boolean(0))
		adapter_props.Set(ADAPTER_INTERFACE, "Discoverable", dbus.Boolean(0))
		print("HCI0 set as non powered, non discoverable and non pairable")
		subprocess.call(['ifconfig', 'br0', "down"])
		subprocess.call(['brctl', 'delbr', "br0"])
		print("Removed br0 interface")
