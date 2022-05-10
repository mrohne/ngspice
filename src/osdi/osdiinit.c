/*
 * Copyright© 2022 SemiMod UG. All rights reserved.
 */

#include "ngspice/stringutil.h"

#include "ngspice/config.h"
#include "ngspice/devdefs.h"
#include "ngspice/iferrmsg.h"
#include "ngspice/memory.h"
#include "ngspice/ngspice.h"
#include "ngspice/typedefs.h"

#include "osdidefs.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * This function converts the information in (a list of) OsdiParamOpvar in
 * descr->param_opvar to the internal ngspice representation (IFparm).
 */
static int write_param_info(IFparm **dst, const OsdiDescriptor *descr,
                            uint32_t start, uint32_t end) {
  for (uint32_t i = start; i < end; i++) {
    OsdiParamOpvar *para = &descr->param_opvar[i];
    uint32_t num_names = para->num_alias + 1;

    int dataType = IF_ASK;
    if ((para->flags & (uint32_t)PARA_KIND_OPVAR) == 0) {
      dataType |= IF_SET;
    }

    switch (para->flags & PARA_TY_MASK) {
    case PARA_TY_REAL:
      dataType |= IF_REAL;
      break;
    case PARA_TY_INT:
      dataType |= IF_INTEGER;
      break;
    case PARA_TY_STR:
      dataType |= IF_STRING;
      break;
    default:
      errRtn = "get_osdi_info";
      errMsg = tprintf("Unkown OSDI type %d for parameter %s!",
                       para->flags & PARA_TY_MASK, para->name[0]);
      return -1;
    }

    if (para->len != 0) {
      dataType |= IF_VECTOR;
    }

    for (uint32_t j = 0; j < num_names; j++) {
      if (j != 0) {
        dataType = IF_UNINTERESTING;
      }
      char *para_name = copy(para->name[j]);
      strtolower(para_name);
      (*dst)[j] = (IFparm){.keyword = para_name,
                           .id = (int)i,
                           .description = para->description,
                           .dataType = dataType};
    }
    *dst += num_names;
  }

  return 0;
}
/**
 * This function creates a SPICEdev instance for a specific OsdiDescriptor by
 * populating the SPICEdev struct with descriptor specific metadata and pointers
 * to the descriptor independent functions.
 * */
extern SPICEdev *osdi_create_spicedev(const OsdiRegistryEntry *entry) {
  const OsdiDescriptor *descr = entry->descriptor;

  // allocate and fill terminal names array
  char **termNames = TMALLOC(char *, descr->num_terminals);
  for (uint32_t i = 0; i < descr->num_terminals; i++) {
    termNames[i] = descr->nodes[i].name;
  }

  // allocate and fill instance params (and opvars)
  int *num_instance_para_names = TMALLOC(int, 1);
  for (uint32_t i = 0; i < descr->num_instance_params; i++) {
    *num_instance_para_names += (int)(1 + descr->param_opvar[i].num_alias);
  }
  for (uint32_t i = descr->num_params;
       i < descr->num_opvars + descr->num_params; i++) {
    *num_instance_para_names += (int)(1 + descr->param_opvar[i].num_alias);
  }
  if (entry->dt != UINT32_MAX) {
    *num_instance_para_names += 1;
  }

  if (entry->temp != UINT32_MAX) {
    *num_instance_para_names += 1;
  }

  IFparm *instance_para_names = TMALLOC(IFparm, *num_instance_para_names);
  IFparm *dst = instance_para_names;

  if (entry->dt != UINT32_MAX) {
    dst[0] = (IFparm){"dt", (int)entry->dt, IF_REAL | IF_SET,
                      "Instance delta temperature"};
    dst += 1;
  }

  if (entry->temp != UINT32_MAX) {
    dst[0] = (IFparm){"temp", (int)entry->temp, IF_REAL | IF_SET,
                      "Instance temperature"};
    dst += 1;
  }
  write_param_info(&dst, descr, 0, descr->num_instance_params);
  write_param_info(&dst, descr, descr->num_params,
                   descr->num_params + descr->num_opvars);

  // allocate and fill model params
  int *num_model_para_names = TMALLOC(int, 1);
  for (uint32_t i = descr->num_instance_params; i < descr->num_params; i++) {
    *num_model_para_names += (int)(1 + descr->param_opvar[i].num_alias);
  }
  IFparm *model_para_names = TMALLOC(IFparm, *num_model_para_names);
  dst = model_para_names;
  write_param_info(&dst, descr, descr->num_instance_params, descr->num_params);

  // Allocate SPICE device
  SPICEdev *OSDIinfo = TMALLOC(SPICEdev, 1);

  // fill information
  OSDIinfo->DEVpublic = (IFdevice){
      .name = descr->name,
      .description = "A simulator independent device loaded with OSDI",
      // TODO why extra indirection? Optional ports?
      .terms = (int *)&descr->num_terminals,
      .numNames = (int *)&descr->num_terminals,
      .termNames = termNames,
      .numInstanceParms = num_instance_para_names,
      .instanceParms = instance_para_names,
      .numModelParms = num_model_para_names,
      .modelParms = model_para_names,
      .flags = DEV_DEFAULT,
      .registry_entry = (void *)entry,
  };

  size_t inst_off = entry->inst_offset;

  int *inst_size = TMALLOC(int, 1);
  *inst_size =
      (int)(inst_off + descr->instance_size + sizeof(OsdiExtraInstData));
  OSDIinfo->DEVinstSize = inst_size;

  size_t model_off = osdi_model_data_off();
  int *model_size = TMALLOC(int, 1);
  *model_size = (int)(model_off + descr->model_size);
  OSDIinfo->DEVmodSize = model_size;

  // fill generic functions
  OSDIinfo->DEVparam = OSDIparam;
  OSDIinfo->DEVmodParam = OSDImParam;
  OSDIinfo->DEVsetup = OSDIsetup;
  OSDIinfo->DEVask = OSDIask;
  OSDIinfo->DEVtemperature = OSDItemp;
  OSDIinfo->DEVload = OSDIload;
  OSDIinfo->DEVacLoad = OSDIacLoad;
  OSDIinfo->DEVunsetup = OSDIunsetup;

  return OSDIinfo;
}

extern FILE *osdi_init_log_message(void *handle_, uint32_t lvl) {
  OsdiNgspiceHandle *handle = handle_;
  switch (lvl) {
  case LOG_LVL_DEBUG:
    fprintf(stdout, "OSDI(debug) %s: ", handle->name);
    break;
  case LOG_LVL_INFO:
    fprintf(stdout, "OSDI(info) %s: ", handle->name);
    break;
  case LOG_LVL_WARN:
    fprintf(stdout, "OSDI(warn) %s: ", handle->name);
    break;
  case LOG_LVL_ERR:
    fprintf(stderr, "OSDI(err) %s: ", handle->name);
    return stderr;
  case LOG_LVL_FATAL:
    fprintf(stderr, "OSDI(fatal) %s: ", handle->name);
    return stderr;
  default:
    fprintf(stdout, "OSDI(unkown) %s", handle->name);
    break;
  }
  return stdout;
}

extern void osdi_finish_log_message(void *handle, FILE *stream, uint32_t lvl) {

  NG_IGNORE(handle);
  NG_IGNORE(stream);
  NG_IGNORE(lvl);
}
