/* see https://john.nachtimwald.com/2014/07/12/wrapping-a-c-library-in-lua/ */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include "luabluez.h"

#define SCANPASSIVE 0x00
#define SCANACTIVE  0x01
#define LEADDR_PUBLIC  0x00
#define LEADDR_RANDOM  0x01
#define LEADDR_PRIV1   0x02
#define LEADDR_PRIV2   0x03
#define FILTERNONE  0x00
#define FILTER_ACCEPT_WHITELISTED_ONLY 0x01

#define SCAN_DISABLE 0x00
#define SCAN_ENABLE  0x01
#define FILTERDUPS_DISABLE 0x00
#define FILTERDUPS_ENABLE  0x01

#define LETIMEOUT  10000

static int
lhci_get_route(lua_State *L) {
  int adapter_id;
  /* does not require an open device */
  /* hci_get_route apparently returns the "resource number" (handle?)
   * of any bt adapter that does NOT match the arg */
  adapter_id = hci_get_route(NULL);
  if (adapter_id < 0) {
    perror("Device is not available");
    exit(1);
  }
  lua_pushinteger(L, adapter_id);
  return 1;
}

static int			/* BT Core vol 2 part E chap 7.8.10 */
lhci_open_dev(lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  luaL_argcheck(L, n >= 1, 1, "at least adapter id arg expected");
  int adapter_id = luaL_checkinteger(L, 1);

  /* FIXME: move sock open out of this func */
  printf("opening socket on adapter %d\n", adapter_id);
  int sock = hci_open_dev(adapter_id);
  if (sock < 0)
    perror("Could not open device");
  lua_pushinteger(L, sock);
  return 1;
}

static int			/* BT Core vol 2 part E chap 7.8.10 */
lhci_le_set_scan_params(lua_State *L) {
			/* int adapter_id, */
			/* uint8_t scan_type, /\* 0x00 Passive (default), 0x01 Active *\/ */
			/* uint16_t interval, /\* 0xXXXX * 0.625 usec, default 0x0010, [0x0004..0x4000] *\/ */
			/* uint16_t window,   /\* scan duration; same description as for interval *\/ */
			/* uint8_t own_type,  /\* own address 0x00 public, 0x01 random, 0x02, 0x03 private) *\/ */
			/* uint8_t filter_policy,	   /\* filter policy: accept*\/ */
			/* /\* 0x00 all except directed ad packets not addressed to me (default) *\/ */
			/* /\* 0x01 only whitelisted advertisers, unless directed elsewhere *\/ */
			/* /\* 0x02 all undirected ads, directed ads to me or from private addr *\/ */
			/* /\* 0x03 whitelisted advertisers, directed ads to me or from private addr *\/ */
			/* /\* uint8_t filter_type /\\* general | limited *\\/ *\/ */
			/* /\*     see BT Core vol 3 part C chapt 9 *\/ */
			/* int timeout /\* in millis, hcitool uses 10000  *\/ */

  int n = lua_gettop(L);  /* number of arguments */
  luaL_argcheck(L, n >= 1, 1, "at least adapter id arg expected");
  /* precon:  first arg is opened socket on bt adapter */
  int sock = luaL_checkinteger(L, 1);
  int err;
  if (n == 1) {		  /* default vals for everything except adapter */
    printf("setting scan params on socket %d\n", sock);
    err = hci_le_set_scan_parameters(sock,
				     SCANPASSIVE, /* scan_type, */
				     0x0010,	  /* interval, */
				     0x0010,	  /* window, */
				     LEADDR_PUBLIC, /* own_type, */
				     0x00, /* filter_policy, */
				     10000 /* timeout */
				     );
    printf("set scan parms result: %x, errno: %x\n", err, errno);
    if (err < 0) {
      perror("set scan params failure");
      exit(1);
    } else {
      printf("set scan params success\n");
    }
    lua_pushinteger(L, sock);

  } else {

  }
  /* int hci_le_set_scan_parameters(int dev_id, uint8_t type, uint16_t interval, */
  /* 					uint16_t window, uint8_t own_type, */
  /* 					uint8_t filter, int to); */
  return 1;
}


int			/* BT Core vol 2 part E chap 7.8.11 */
lhci_le_set_scan_enable(lua_State *L) {
/* int hci_le_set_scan_enable(int dev_id, uint8_t enable, uint8_t filter_dup, int to); */
/* parameters: */
/*     adapter: socket (int) */
/*     LE_Scan_Enable: uint8_t (boolean), starts/stops scanning */
/*     Filter_Duplicates: uint8_t (boolean), controls link layer filtering of duplicate adv. reports */
/*     timeout: int */

 /* preconditions:  arg 1 is socket opened on adapter */
  printf("lhci_le_set_scan_enable\n");
  int n = lua_gettop(L);  /* number of arguments */
  luaL_argcheck(L, n >= 1, 1, "at least adapter socket arg expected");
  int sock = luaL_checkinteger(L, 1);
  int err;

  printf("dis/enabling scan on socket %d\n", sock);
  if (n == 1) {		  /* default vals for everything except adapter socket */
    /* FIXME: default:  check state, then toggle */
    int err = hci_le_set_scan_enable(sock, SCAN_ENABLE, FILTERDUPS_ENABLE, LETIMEOUT);
    printf("scan enable result: %d\n", err);
    if (err < 0) {
      perror("Could not enable scanning");
      exit(1);
    }
    lua_pushinteger(L, err);
  } else {
    int ena  = luaL_checkinteger(L, 2);
    printf("enabling? %x\n", ena);
    int err = hci_le_set_scan_enable(sock, ena, FILTERDUPS_ENABLE, LETIMEOUT);
    printf("2 scan enable result: %d\n", err);
    if (err < 0) {
      perror("2 Could not enable scanning");
      exit(1);
    }
    lua_pushinteger(L, err);
  }
  /* int hci_le_set_scan_parameters(int dev_id, uint8_t type, uint16_t interval, */
  /* 					uint16_t window, uint8_t own_type, */
  /* 					uint8_t filter, int to); */
  return 1;

}



/* int hci_le_set_advertise_enable(int dev_id, uint8_t enable, int to); */
/* int hci_le_create_conn(int dd, uint16_t interval, uint16_t window, */
/* 		uint8_t initiator_filter, uint8_t peer_bdaddr_type, */
/* 		bdaddr_t peer_bdaddr, uint8_t own_bdaddr_type, */
/* 		uint16_t min_interval, uint16_t max_interval, */
/* 		uint16_t latency, uint16_t supervision_timeout, */
/* 		uint16_t min_ce_length, uint16_t max_ce_length, */
/* 		uint16_t *handle, int to); */
/* int hci_le_conn_update(int dd, uint16_t handle, uint16_t min_interval, */
/* 			uint16_t max_interval, uint16_t latency, */
/* 			uint16_t supervision_timeout, int to); */
/* int hci_le_add_white_list(int dd, const bdaddr_t *bdaddr, uint8_t type, int to); */
/* int hci_le_rm_white_list(int dd, const bdaddr_t *bdaddr, uint8_t type, int to); */
/* int hci_le_read_white_list_size(int dd, uint8_t *size, int to); */
/* int hci_le_clear_white_list(int dd, int to); */
/* int hci_le_add_resolving_list(int dd, const bdaddr_t *bdaddr, uint8_t type, */
/* 				uint8_t *peer_irk, uint8_t *local_irk, int to); */
/* int hci_le_rm_resolving_list(int dd, const bdaddr_t *bdaddr, uint8_t type, int to); */
/* int hci_le_clear_resolving_list(int dd, int to); */
/* int hci_le_read_resolving_list_size(int dd, uint8_t *size, int to); */
/* int hci_le_set_address_resolution_enable(int dev_id, uint8_t enable, int to); */
/* int hci_le_read_remote_features(int dd, uint16_t handle, uint8_t *features, int to); */



static int lhci_new(lua_State *L)
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

static int lhci_destroy(lua_State *L)
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

static const struct luaL_Reg lhci_methods[] = {
  { "__gc",        lhci_destroy   },
  /* { "__tostring",  lhci_tostring  }, */
  { NULL,          NULL               },
};

static const struct luaL_Reg lhci_functions[] = {
  { "new", lhci_new },
  { "getadapter", lhci_get_route },
  { "openadapter", lhci_open_dev },
  { "set_scan_params", lhci_le_set_scan_params },
  { "lescan", lhci_le_set_scan_enable },
  { NULL,  NULL         }
};

int luaopen_luahci(lua_State *L)
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
  luaL_setfuncs(L, lhci_methods, 0);

  /* Register the object.func functions into the table that is at the top of the
   * stack. */
  luaL_newlib(L, lhci_functions);

  return 1;
}
