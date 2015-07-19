# luabluez
lua wrapper of [bluez](http://www.bluez.org) apis (bluetooth stack)

# overview

The Bluez stack supports a variety of APIs; luabluez currently only
wraps (part of) the HCI API.

# getting started

Bluez comes with several tools that can be used to configure, test,
and generally explore Bluetooth (classic and le) from the command
line.  The ones you will probably use the most are `bluetoothctl`,
`hciconfig` and `hcitool`.

Additional tools are in the [bluez-tools](https://code.google.com/p/bluez-tools/) package.

 The following docs from Intel show how to use some of the bluez tools
with the Edison:

* [Intel® Edison Board Getting Started with Bluetooth](https://software.intel.com/en-us/articles/intel-edison-board-getting-started-with-bluetooth)
* [Edison Bluetooth Guide](https://software.intel.com/en-us/articles/intel-edison-bluetooth-guide)

In brief:

* `$ rfkill list`
* `$ rfkill unblock bluetooth` (or block)
* `$ hciconfig`
* `$ hciconfig hci0 up` (or down)
* `$ hciconfig --help`
* `$ hcitool --help`

To use bluetoothctl run `$ bluetoothctl` and then issue the `help`
subcommand.  The usual protocol is to start bluetoothctl and then do:

* `[bluetooth]# agent KeyboardDisplay`
* `[bluetooth]# default-agent`

Memorize these two subcommands; if you don't issue them you won't see output.

Classic scan:

* `[bluetooth]# scan on`
* `[bluetooth]# scan off`

Sometimes scanning with bluetoothctl does not seem to work; the
alternative is `$ hcitool scan`

Bluetooth LE scan: `$ hcitool lescan`


## acronyms

The Bluetooth Core spec contains a long table listing acronyms.  Some relevant ones:

* ATT
* GAP
* GATT
* HCI - Host Controller Interface - defined in the Bluetooth Core spec
* L2CAP
* RFCOMM
* SDP - Service Discovery Protocol

# bluez

*CAVEAT*: bluez 5 involved major breaking changes.  Lots of
 documentation on the use of bluez available on the web only applies
 to previous versions.  For example, you can find instructions on how
 to use `bluez-simple-agent`; this was replaced in bluez5 by
 `bluetoothctl`.  Bluez5 was released in late 2012 (see the
 [Intro and Porting Guide](http://www.bluez.org/bluez-5-api-introduction-and-porting-guide/),
 so check the date on any documentation you find online.

## tools

The stack includes several useful tools that you can use to get started:

* bluetoothctl - uses dbus API (source in client/)
* gatttool - not installed by default on the Edison; you must download the bluez source and compile it on the Edison (source in attrib/)
* hciconfig - uses the hci API (source in tools/)
* hcitool - uses the hci API (source in tools/)

See also [How to run bluez](http://www.stlinux.com/kernel/bluetooth/how-to-run-BlueZ)

The tools/ subdir contains a variety of tools.  Some of them are
generally useful; others are specific to particular products
(e.g. CSR):

* 3dsp - 3D Synchronization Profile testing
* amptest
* avinfo - Audio/Video Info Tool
* avtest
* bdaddr - change (local) Bluetooth device address
* bluetooth-player
* bnep-tester - BNEP = Bluetooth Network Encapsulation Protocol
* bneptest
* btattach
* btgatt-client
* btgatt-server
* btinfo
* btiotest
* btmgmt
* btproxy
* btsnoop
* check-selftest
* ciptool - CIP = Bluetooth Common ISDN Access Profile
* cltest
* create-image - CPIO image creation utility
* gap-tester
* gatt-service
* hci-tester - HCI = [Host Controller Interface](https://developer.bluetooth.org/TechnologyOverview/Pages/HCI.aspx)
* hciattach
* hciconfig
* hcidump
* hcieventmask
* hcisecfilter
* hcitool
* hwdb - hardware vendor list (from [](http://www.bluetooth.org/Technical/AssignedNumbers/identifiers.htm))
* ibeacon
* l2ping - L2CAP = [Logical Link Control and Adaptation](https://developer.bluetooth.org/TechnologyOverview/Pages/L2CAP.aspx)
* l2test - L2CAP testing
* mcaptest - MCAP = [Multi-Channel Adaptation Protocol](https://developer.bluetooth.org/TechnologyOverview/Pages/MCAP.aspx)
* mgmt-tester
* mpris-proxy - MPRIS = [Media Player Remote Interfacing Specification](http://specifications.freedesktop.org/mpris-spec/latest/) - a standard DBUS interface "which aims to provide a common programmatic API for controlling media players."
* obex-client-tool
* obex-server-tool
* obexctl
* oobtest - Out-of-band pairing testing
* parser
* rctest - RFCOMM testing; "The [RFCOMM protocol](https://developer.bluetooth.org/TechnologyOverview/Pages/RFCOMM.aspx) emulates the serial cable line settings and status of an RS-232 serial port and is used for providing serial data transfer."
* rfcomm - RFCOMM configuration utility
* rfcomm-tester
* sco-tester - SCO = Synchronous Connection-Oriented [logical transport]
* scotest
* sdptool - SDP = [Service Discovery Protocol](https://www.bluetooth.org/en-us/specification/assigned-numbers/service-discovery)
* smp-tester
* test-runner
* userchan-tester

Vendor-specific tools in tools/:
* bccmd - Utility for the [CSR](http://www.csr.com/products/technologies/bluetooth-smart) BCCMD interface.
* bluemoon - Bluemoon configuration utility
* hex2hcd - Broadcom Bluetooth firmware converter
* nokfw - Nokia Bluetooth firmware analyzer
* seq2bseq - Intel Bluetooth firmware converter

## code and apis

The bluez stack involves several apis:

* [dbus](http://www.freedesktop.org/wiki/Software/dbus/) - standard Linux IPC mechanism
* hci - Host Controller Interface, defined by Bluetooth
* sdp - Service Discovery Protocol, defined by Bluetooth
* etc.

There is no user-level documentation explaining the APIs, but once you
learn how to use bluetoothctl, hciconfig, hcitool, etc. you can
examine the source code to see how the APIs work.  The source tree
looks something like the following.

* attrib/ - code for gatttool
* client/ - code for the bluetoothctl tool (note there is no bluetoothctl.c file)
* lib/ - lower-level api implementations - hci, sdp, etc.
* src/ - implementations for the dbus api?
* unit/ - various C programs for unit testing

The preferred application-level API seems to be dbus, but as of July
2015 that API seems to be not quite complete.  It is documented in the
docs/ subdir.

# Resources

## Bluetooth
* [Bluetooth `Adopted' Specifications](https://www.bluetooth.org/en-us/specification/adopted-specifications?_ga=1.178563111.220673070.1436421945) - you'll want Core Version 4.2 or later

## Intel
* [Intel® Edison Board Getting Started with Bluetooth](https://software.intel.com/en-us/articles/intel-edison-board-getting-started-with-bluetooth)
* [Edison Bluetooth Guide](https://software.intel.com/en-us/articles/intel-edison-bluetooth-guide)
