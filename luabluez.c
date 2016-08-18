/* see https://john.nachtimwald.com/2014/07/12/wrapping-a-c-library-in-lua/ */

#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "luabluez.h"

/* Userdata object that will hold the luabluez and name. */
/* typedef struct { */
/*   luabluez_t *c; */
/*   char      *name; */
/* } luabluez_userdata_t; */

#define for_each_opt(opt, long, short) while ((opt=getopt_long(argc, argv, short ? short:"+", long, NULL)) != -1)

static void
cmd_lescan(int adapter_id, int argc, char **argv) {
}

static void
bt_scan(int adapter_id, int argc, char **argv) {
  inquiry_info *info = NULL;
  int max_rsp, num_rsp, length, flags;
  uint8_t lap[3] = { 0x33, 0x8b, 0x9e };
  uint8_t cls[3], features[8];
  char addr[18], name[249], *comp;
  struct hci_version version;
  struct hci_dev_info di;
  struct hci_conn_info_req *cr;
  int extcls = 0, extinf = 0, extoui = 0;
  int i, n, l, opt, dd, cc;

  length  = 8;	/* ~10 seconds */
  num_rsp = 0;
  flags   = 0;

  /* Huang steps: */
  /*     1. hci_get_route - obtain bdaddr for local bt adapter */
  /*     2. hci_open_dev  - open a socket on it */
  /*     3. hci_inquiry */
  /*     4. for each dev listed by inquiry: */
  /*     5. hci_read_remote_name */

  /* NB: hci_inquiry does: hci_get_route, open socket, then ioctl
   * HCIINQUIRY, so we don't need to open it in order to do an
   * inquiry? */

  if (adapter_id < 0) {
    adapter_id = hci_get_route(NULL);
    if (adapter_id < 0) {
      perror("No enabled local adapter found");
      exit(1);
    }
  }

  /* hci_open_dev() */

  /* FIXME: hci_devinfo doesn't belong in scan func */
  /* hci_devinfo does ioctl HCIGETDEVINFO and checks bit HCI_RAW */
  if (hci_devinfo(adapter_id, &di) < 0) {
    perror("Can't get device info");
    exit(1);
  }

  printf("Scanning ...\n");
  num_rsp = hci_inquiry(adapter_id, length, num_rsp, lap, &info, flags);
  if (num_rsp < 0) {
    perror("Inquiry failed");
    exit(1);
  }

  dd = hci_open_dev(adapter_id);
  if (dd < 0) {
    perror("HCI device open failed");
    free(info);
    exit(1);
  }

  if (extcls || extinf || extoui)
    printf("\n");

  for (i = 0; i < num_rsp; i++) {
    uint16_t handle = 0;

    if (!extcls && !extinf && !extoui) {
      ba2str(&(info+i)->bdaddr, addr);

      if (hci_read_remote_name_with_clock_offset(dd,
						 &(info+i)->bdaddr,
						 (info+i)->pscan_rep_mode,
						 (info+i)->clock_offset | 0x8000,
						 sizeof(name), name, 100000) < 0)
	strcpy(name, "n/a");

      for (n = 0; n < 248 && name[n]; n++) {
	if ((unsigned char) name[i] < 32 || name[i] == 127)
	  name[i] = '.';
      }

      name[248] = '\0';

      printf("\t%s\t%s\n", addr, name);
      continue;
    }

    ba2str(&(info+i)->bdaddr, addr);
    printf("BD Address:\t%s [mode %d, clkoffset 0x%4.4x]\n", addr,
	   (info+i)->pscan_rep_mode, btohs((info+i)->clock_offset));

    if (extoui) {
      comp = batocomp(&(info+i)->bdaddr);
      if (comp) {
	char oui[9];
	ba2oui(&(info+i)->bdaddr, oui);
	printf("OUI company:\t%s (%s)\n", comp, oui);
	free(comp);
      }
    }

    cc = 0;

    if (extinf) {
      cr = malloc(sizeof(*cr) + sizeof(struct hci_conn_info));
      if (cr) {
	bacpy(&cr->bdaddr, &(info+i)->bdaddr);
	cr->type = ACL_LINK;
	if (ioctl(dd, HCIGETCONNINFO, (unsigned long) cr) < 0) {
	  handle = 0;
	  cc = 1;
	} else {
	  handle = htobs(cr->conn_info->handle);
	  cc = 0;
	}
	free(cr);
      }

      if (cc) {
	if (hci_create_connection(dd, &(info+i)->bdaddr,
				  htobs(di.pkt_type & ACL_PTYPE_MASK),
				  (info+i)->clock_offset | 0x8000,
				  0x01, &handle, 25000) < 0) {
	  handle = 0;
	  cc = 0;
	}
      }
    }

    if (hci_read_remote_name_with_clock_offset(dd,
					       &(info+i)->bdaddr,
					       (info+i)->pscan_rep_mode,
					       (info+i)->clock_offset | 0x8000,
					       sizeof(name), name, 100000) < 0) {
    } else {
      for (n = 0; n < 248 && name[n]; n++) {
	if ((unsigned char) name[i] < 32 || name[i] == 127)
	  name[i] = '.';
      }

      name[248] = '\0';
    }

    if (strlen(name) > 0)
      printf("Device name:\t%s\n", name);

    if (extcls) {
      memcpy(cls, (info+i)->dev_class, 3);
      printf("Device class:\t");
      if ((cls[1] & 0x1f) > sizeof(major_classes) / sizeof(char *))
	printf("Invalid");
      else
	printf("%s, %s", major_classes[cls[1] & 0x1f],
	       get_minor_device_name(cls[1] & 0x1f, cls[0] >> 2));
      printf(" (0x%2.2x%2.2x%2.2x)\n", cls[2], cls[1], cls[0]);
    }

    if (extinf && handle > 0) {
      if (hci_read_remote_version(dd, handle, &version, 20000) == 0) {
	char *ver = lmp_vertostr(version.lmp_ver);
	printf("Manufacturer:\t%s (%d)\n",
	       bt_compidtostr(version.manufacturer),
	       version.manufacturer);
	printf("LMP version:\t%s (0x%x) [subver 0x%x]\n",
	       ver ? ver : "n/a",
	       version.lmp_ver, version.lmp_subver);
	if (ver)
	  bt_free(ver);
      }

      if (hci_read_remote_features(dd, handle, features, 20000) == 0) {
	char *tmp = lmp_featurestostr(features, "\t\t", 63);
	printf("LMP features:\t0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x"
	       " 0x%2.2x 0x%2.2x 0x%2.2x 0x%2.2x\n",
	       features[0], features[1],
	       features[2], features[3],
	       features[4], features[5],
	       features[6], features[7]);
	printf("%s\n", tmp);
	bt_free(tmp);
      }

      if (cc) {
	usleep(10000);
	hci_disconnect(dd, handle, HCI_OE_USER_ENDED_CONNECTION, 10000);
      }
    }

    printf("\n");
  }

  bt_free(info);

  hci_close_dev(dd);
}



static int lbluez_new(lua_State *L)
{
  /* luabluez_userdata_t *cu; */
  const char          *name;
  int                  start;

  /* Check the arguments are valid. */
  start = luaL_checkinteger(L, 1);
  name  = luaL_checkstring(L, 2);
  if (name == NULL)
    luaL_error(L, "name cannot be empty");

  /* Create the user data pushing it onto the stack. We also pre-initialize
   * the member of the userdata in case initialization fails in some way. If
   * that happens we want the userdata to be in a consistent state for __gc. */
  /* cu       = (luabluez_userdata_t *)lua_newuserdata(L, sizeof(*cu)); */
  /* cu->c    = NULL; */
  /* cu->name = NULL; */

  /* Add the metatable to the stack. */
  luaL_getmetatable(L, "Luabluez");
  /* Set the metatable on the userdata. */
  lua_setmetatable(L, -2);

  /* Create the data that comprises the userdata (the luabluez). */
  /* cu->c    = luabluez_create(start); */
  /* cu->name = strdup(name);	/\* hack *\/ */

  return 1;
}

static int lbluez_destroy(lua_State *L)
{
  /* luabluez_userdata_t *cu; */

  /* cu = (luabluez_userdata_t *)luaL_checkudata(L, 1, "Luabluez"); */

  /* if (cu->c != NULL) */
  /*   luabluez_destroy(cu->c); */
  /* cu->c = NULL; */

  /* if (cu->name != NULL) */
  /*   free(cu->name); */
  /* cu->name = NULL; */

  return 0;
}

static int lbluez_tostring(lua_State *L)
{
  /* luabluez_userdata_t *cu; */

  /* cu = (luabluez_userdata_t *)luaL_checkudata(L, 1, "Luabluez"); */

  /* lua_pushfstring(L, "%s(%d)", cu->name, luabluez_getval(cu->c)); */

  return 1;
}

static const struct luaL_Reg lbluez_methods[] = {
  { "__gc",        lbluez_destroy   },
  { "__tostring",  lbluez_tostring  },
  { NULL,          NULL               },
};

static const struct luaL_Reg lbluez_functions[] = {
  { "new", lbluez_new },
  { NULL,  NULL         }
};

int luaopen_luabluez(lua_State *L)
{
  /* Create the metatable and put it on the stack. */
  luaL_newmetatable(L, "Luabluez");
  /* Duplicate the metatable on the stack (We know have 2). */
  lua_pushvalue(L, -1);
  /* Pop the first metatable off the stack and assign it to __index
   * of the second one. We set the metatable for the table to itself.
   * This is equivalent to the following in lua:
   * metatable = {}
   * metatable.__index = metatable
   */
  lua_setfield(L, -2, "__index");

  /* Set the methods to the metatable that should be accessed via object:func */
  luaL_setfuncs(L, lbluez_methods, 0);

  /* Register the object.func functions into the table that is at the top of the
   * stack. */
  luaL_newlib(L, lbluez_functions);

  return 1;
}
