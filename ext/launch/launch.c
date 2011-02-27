#include <ruby.h>
#include <launch.h>
#include <errno.h>
#include <stdio.h>

VALUE mLaunch;
VALUE mLaunchKey;
VALUE mLaunchJobKey;
VALUE mLaunchSocket;
VALUE mLaunchJobPolicy;
VALUE mLaunchData;
VALUE cLaunchError;

static VALUE launch_data_to_ruby(launch_data_t);

static void
launch_dict_iterator(launch_data_t item, const char * key, void * hash) {
  rb_hash_aset((VALUE)hash, rb_str_new2(key), launch_data_to_ruby(item));
}

static VALUE
launch_data_to_ruby(launch_data_t item) {
  VALUE out = Qnil;
  launch_data_t temp = NULL;
  launch_data_type_t item_type = launch_data_get_type(item);
  int i = 0;
  size_t length = 0;

  switch(item_type) {
    case LAUNCH_DATA_DICTIONARY:
      out = rb_hash_new();

      launch_data_dict_iterate(item, launch_dict_iterator, (void *)out);

      break;
	  case LAUNCH_DATA_ARRAY:
      length = launch_data_array_get_count(item);
      out = rb_ary_new2(length);

      for (i = 0; i < length; i++) {
        temp = launch_data_array_get_index(item, i);
        rb_ary_store(out, i, launch_data_to_ruby(temp));
      }

      break;
	  case LAUNCH_DATA_FD:
      out = INT2NUM(launch_data_get_fd(item));
      break;
	  case LAUNCH_DATA_INTEGER:
      out = LONG2NUM(launch_data_get_integer(item));
      break;
	  case LAUNCH_DATA_REAL:
      out = DBL2NUM(launch_data_get_real(item));
      break;
	  case LAUNCH_DATA_BOOL:
      out = launch_data_get_bool(item) == TRUE ? Qtrue : Qfalse;
      break;
	  case LAUNCH_DATA_STRING:
      out = rb_str_new2(launch_data_get_string(item));
      break;
	  case LAUNCH_DATA_OPAQUE:
      out = rb_str_new(launch_data_get_opaque(item),
          launch_data_get_opaque_size(item));
      break;
	  case LAUNCH_DATA_ERRNO:
      out = INT2NUM(launch_data_get_errno(item));
      break;
	  case LAUNCH_DATA_MACHPORT:
      break;
    default:
      rb_raise(cLaunchError, "unknown item type %d", item_type);
  }

  return out;
}

/*
 * call-seq:
 *   launch_message(Launch::Key::CHECKIN) # => response
 *
 * launch_message wraps launch(3)'s launch_msg function.  The argument is a
 * message to send launchd from Launch::Key.  Please use launch_checkin
 * and launch_sockets instead of this method.
 *
 * The response from launchd is converted to a ruby structure and freed
 * (usually a Hash).  Currently the following data types in the response are
 * indistinguishable in the ruby structure:
 *
 * * STRING and OPAQUE
 * * INTEGER, FD and ERRNO
 *
 * Additionally, MACHPORT is ignored.
 *
 * If there was a failure to retrieve checkin data an error will be raised.
 */
static VALUE
message(VALUE self, VALUE message) {
  VALUE out, tmp;
  launch_data_t send, response;
  launch_data_type_t response_type;

  message = rb_str_to_str(message);

  send = launch_data_new_string(StringValueCStr(message));

  if (send == NULL) {
    tmp = rb_inspect(message);
    rb_raise(cLaunchError, "unable to create message %s",
        StringValueCStr(tmp));
  }

  response = launch_msg(send);

  if (response == NULL)
    rb_sys_fail("launch_msg");

  response_type = launch_data_get_type(response);

  if (response_type == LAUNCH_DATA_ERRNO) {
    errno = launch_data_get_errno(response);
    rb_sys_fail("launch_msg");
  }

  out = launch_data_to_ruby(response);

  launch_data_free(response);

  return out;
}

void
Init_launch() {
  mLaunch = rb_define_module("Launch");
  cLaunchError = rb_define_class_under(mLaunch, "Error", rb_eRuntimeError);

  rb_define_method(mLaunch, "launch_message", message, 1);

  rb_const_set(mLaunch, rb_intern("JOBINETDCOMPATIBILITY_WAIT"),
      rb_str_new2(LAUNCH_JOBINETDCOMPATIBILITY_WAIT));

  /*
   * Launch::Key holds launch message types
   */
  mLaunchKey = rb_define_module_under(mLaunch, "Key");
  rb_const_set(mLaunchKey, rb_intern("SUBMITJOB"),
        rb_str_new2(LAUNCH_KEY_SUBMITJOB));
  rb_const_set(mLaunchKey, rb_intern("REMOVEJOB"),
      rb_str_new2(LAUNCH_KEY_REMOVEJOB));
  rb_const_set(mLaunchKey, rb_intern("STARTJOB"),
      rb_str_new2(LAUNCH_KEY_STARTJOB));
  rb_const_set(mLaunchKey, rb_intern("STOPJOB"),
      rb_str_new2(LAUNCH_KEY_STOPJOB));
  rb_const_set(mLaunchKey, rb_intern("GETJOB"),
      rb_str_new2(LAUNCH_KEY_GETJOB));
  rb_const_set(mLaunchKey, rb_intern("GETJOBS"),
      rb_str_new2(LAUNCH_KEY_GETJOBS));
  rb_const_set(mLaunchKey, rb_intern("CHECKIN"),
      rb_str_new2(LAUNCH_KEY_CHECKIN));

  /*
   * Launch::JobKey holds various keys returned in a message response
   */
  mLaunchJobKey = rb_define_module_under(mLaunch, "JobKey");
  rb_const_set(mLaunchJobKey, rb_intern("LABEL"),
      rb_str_new2(LAUNCH_JOBKEY_LABEL));
  rb_const_set(mLaunchJobKey, rb_intern("DISABLED"),
      rb_str_new2(LAUNCH_JOBKEY_DISABLED));
  rb_const_set(mLaunchJobKey, rb_intern("USERNAME"),
      rb_str_new2(LAUNCH_JOBKEY_USERNAME));
  rb_const_set(mLaunchJobKey, rb_intern("GROUPNAME"),
      rb_str_new2(LAUNCH_JOBKEY_GROUPNAME));
  rb_const_set(mLaunchJobKey, rb_intern("TIMEOUT"),
      rb_str_new2(LAUNCH_JOBKEY_TIMEOUT));
  rb_const_set(mLaunchJobKey, rb_intern("EXITTIMEOUT"),
      rb_str_new2(LAUNCH_JOBKEY_EXITTIMEOUT));
  rb_const_set(mLaunchJobKey, rb_intern("INITGROUPS"),
      rb_str_new2(LAUNCH_JOBKEY_INITGROUPS));
  rb_const_set(mLaunchJobKey, rb_intern("SOCKETS"),
      rb_str_new2(LAUNCH_JOBKEY_SOCKETS));
  rb_const_set(mLaunchJobKey, rb_intern("MACHSERVICES"),
      rb_str_new2(LAUNCH_JOBKEY_MACHSERVICES));
  rb_const_set(mLaunchJobKey, rb_intern("MACHSERVICELOOKUPPOLICIES"),
      rb_str_new2(LAUNCH_JOBKEY_MACHSERVICELOOKUPPOLICIES));
  rb_const_set(mLaunchJobKey, rb_intern("INETDCOMPATIBILITY"),
      rb_str_new2(LAUNCH_JOBKEY_INETDCOMPATIBILITY));
  rb_const_set(mLaunchJobKey, rb_intern("ENABLEGLOBBING"),
      rb_str_new2(LAUNCH_JOBKEY_ENABLEGLOBBING));
  rb_const_set(mLaunchJobKey, rb_intern("PROGRAMARGUMENTS"),
      rb_str_new2(LAUNCH_JOBKEY_PROGRAMARGUMENTS));
  rb_const_set(mLaunchJobKey, rb_intern("PROGRAM"),
      rb_str_new2(LAUNCH_JOBKEY_PROGRAM));
  rb_const_set(mLaunchJobKey, rb_intern("ONDEMAND"),
      rb_str_new2(LAUNCH_JOBKEY_ONDEMAND));
  rb_const_set(mLaunchJobKey, rb_intern("KEEPALIVE"),
      rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE));
  rb_const_set(mLaunchJobKey, rb_intern("LIMITLOADTOHOSTS"),
      rb_str_new2(LAUNCH_JOBKEY_LIMITLOADTOHOSTS));
  rb_const_set(mLaunchJobKey, rb_intern("LIMITLOADFROMHOSTS"),
      rb_str_new2(LAUNCH_JOBKEY_LIMITLOADFROMHOSTS));
  rb_const_set(mLaunchJobKey, rb_intern("LIMITLOADTOSESSIONTYPE"),
      rb_str_new2(LAUNCH_JOBKEY_LIMITLOADTOSESSIONTYPE));
  rb_const_set(mLaunchJobKey, rb_intern("RUNATLOAD"),
      rb_str_new2(LAUNCH_JOBKEY_RUNATLOAD));
  rb_const_set(mLaunchJobKey, rb_intern("ROOTDIRECTORY"),
      rb_str_new2(LAUNCH_JOBKEY_ROOTDIRECTORY));
  rb_const_set(mLaunchJobKey, rb_intern("WORKINGDIRECTORY"),
      rb_str_new2(LAUNCH_JOBKEY_WORKINGDIRECTORY));
  rb_const_set(mLaunchJobKey, rb_intern("ENVIRONMENTVARIABLES"),
      rb_str_new2(LAUNCH_JOBKEY_ENVIRONMENTVARIABLES));
  rb_const_set(mLaunchJobKey, rb_intern("USERENVIRONMENTVARIABLES"),
      rb_str_new2(LAUNCH_JOBKEY_USERENVIRONMENTVARIABLES));
  rb_const_set(mLaunchJobKey, rb_intern("UMASK"),
      rb_str_new2(LAUNCH_JOBKEY_UMASK));
  rb_const_set(mLaunchJobKey, rb_intern("NICE"),
      rb_str_new2(LAUNCH_JOBKEY_NICE));
  rb_const_set(mLaunchJobKey, rb_intern("HOPEFULLYEXITSFIRST"),
      rb_str_new2(LAUNCH_JOBKEY_HOPEFULLYEXITSFIRST));
  rb_const_set(mLaunchJobKey, rb_intern("HOPEFULLYEXITSLAST"),
      rb_str_new2(LAUNCH_JOBKEY_HOPEFULLYEXITSLAST));
  rb_const_set(mLaunchJobKey, rb_intern("LOWPRIORITYIO"),
      rb_str_new2(LAUNCH_JOBKEY_LOWPRIORITYIO));
  rb_const_set(mLaunchJobKey, rb_intern("SESSIONCREATE"),
      rb_str_new2(LAUNCH_JOBKEY_SESSIONCREATE));
  rb_const_set(mLaunchJobKey, rb_intern("STARTONMOUNT"),
      rb_str_new2(LAUNCH_JOBKEY_STARTONMOUNT));
  rb_const_set(mLaunchJobKey, rb_intern("SOFTRESOURCELIMITS"),
      rb_str_new2(LAUNCH_JOBKEY_SOFTRESOURCELIMITS));
  rb_const_set(mLaunchJobKey, rb_intern("HARDRESOURCELIMITS"),
      rb_str_new2(LAUNCH_JOBKEY_HARDRESOURCELIMITS));
  rb_const_set(mLaunchJobKey, rb_intern("STANDARDINPATH"),
      rb_str_new2(LAUNCH_JOBKEY_STANDARDINPATH));
  rb_const_set(mLaunchJobKey, rb_intern("STANDARDOUTPATH"),
      rb_str_new2(LAUNCH_JOBKEY_STANDARDOUTPATH));
  rb_const_set(mLaunchJobKey, rb_intern("STANDARDERRORPATH"),
      rb_str_new2(LAUNCH_JOBKEY_STANDARDERRORPATH));
  rb_const_set(mLaunchJobKey, rb_intern("DEBUG"),
      rb_str_new2(LAUNCH_JOBKEY_DEBUG));
  rb_const_set(mLaunchJobKey, rb_intern("WAITFORDEBUGGER"),
      rb_str_new2(LAUNCH_JOBKEY_WAITFORDEBUGGER));
  rb_const_set(mLaunchJobKey, rb_intern("QUEUEDIRECTORIES"),
      rb_str_new2(LAUNCH_JOBKEY_QUEUEDIRECTORIES));
  rb_const_set(mLaunchJobKey, rb_intern("WATCHPATHS"),
      rb_str_new2(LAUNCH_JOBKEY_WATCHPATHS));
  rb_const_set(mLaunchJobKey, rb_intern("STARTINTERVAL"),
      rb_str_new2(LAUNCH_JOBKEY_STARTINTERVAL));
  rb_const_set(mLaunchJobKey, rb_intern("STARTCALENDARINTERVAL"),
      rb_str_new2(LAUNCH_JOBKEY_STARTCALENDARINTERVAL));
  rb_const_set(mLaunchJobKey, rb_intern("BONJOURFDS"),
      rb_str_new2(LAUNCH_JOBKEY_BONJOURFDS));
  rb_const_set(mLaunchJobKey, rb_intern("LASTEXITSTATUS"),
      rb_str_new2(LAUNCH_JOBKEY_LASTEXITSTATUS));
  rb_const_set(mLaunchJobKey, rb_intern("PID"),
      rb_str_new2(LAUNCH_JOBKEY_PID));
  rb_const_set(mLaunchJobKey, rb_intern("THROTTLEINTERVAL"),
      rb_str_new2(LAUNCH_JOBKEY_THROTTLEINTERVAL));
  rb_const_set(mLaunchJobKey, rb_intern("LAUNCHONLYONCE"),
      rb_str_new2(LAUNCH_JOBKEY_LAUNCHONLYONCE));
  rb_const_set(mLaunchJobKey, rb_intern("ABANDONPROCESSGROUP"),
      rb_str_new2(LAUNCH_JOBKEY_ABANDONPROCESSGROUP));
  rb_const_set(mLaunchJobKey, rb_intern("IGNOREPROCESSGROUPATSHUTDOWN"),
      rb_str_new2(LAUNCH_JOBKEY_IGNOREPROCESSGROUPATSHUTDOWN));
  rb_const_set(mLaunchJobKey, rb_intern("POLICIES"),
      rb_str_new2(LAUNCH_JOBKEY_POLICIES));
  rb_const_set(mLaunchJobKey, rb_intern("ENABLETRANSACTIONS"),
      rb_str_new2(LAUNCH_JOBKEY_ENABLETRANSACTIONS));

  rb_const_set(mLaunchJobKey, rb_intern("MACH_RESETATCLOSE"),
      rb_str_new2(LAUNCH_JOBKEY_MACH_RESETATCLOSE));
  rb_const_set(mLaunchJobKey, rb_intern("MACH_HIDEUNTILCHECKIN"),
      rb_str_new2(LAUNCH_JOBKEY_MACH_HIDEUNTILCHECKIN));
  rb_const_set(mLaunchJobKey, rb_intern("MACH_DRAINMESSAGESONCRASH"),
      rb_str_new2(LAUNCH_JOBKEY_MACH_DRAINMESSAGESONCRASH));

  rb_const_set(mLaunchJobKey, rb_intern("KEEPALIVE_SUCCESSFULEXIT"),
      rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_SUCCESSFULEXIT));
  rb_const_set(mLaunchJobKey, rb_intern("KEEPALIVE_NETWORKSTATE"),
      rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_NETWORKSTATE));
  rb_const_set(mLaunchJobKey, rb_intern("KEEPALIVE_PATHSTATE"),
      rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_PATHSTATE));
  rb_const_set(mLaunchJobKey, rb_intern("KEEPALIVE_OTHERJOBACTIVE"),
      rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_OTHERJOBACTIVE));
  rb_const_set(mLaunchJobKey, rb_intern("KEEPALIVE_OTHERJOBENABLED"),
      rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_OTHERJOBENABLED));
  rb_const_set(mLaunchJobKey, rb_intern("KEEPALIVE_AFTERINITIALDEMAND"),
      rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_AFTERINITIALDEMAND));

  rb_const_set(mLaunchJobKey, rb_intern("CAL_MINUTE"),
      rb_str_new2(LAUNCH_JOBKEY_CAL_MINUTE));
  rb_const_set(mLaunchJobKey, rb_intern("CAL_HOUR"),
      rb_str_new2(LAUNCH_JOBKEY_CAL_HOUR));
  rb_const_set(mLaunchJobKey, rb_intern("CAL_DAY"),
      rb_str_new2(LAUNCH_JOBKEY_CAL_DAY));
  rb_const_set(mLaunchJobKey, rb_intern("CAL_WEEKDAY"),
      rb_str_new2(LAUNCH_JOBKEY_CAL_WEEKDAY));
  rb_const_set(mLaunchJobKey, rb_intern("CAL_MONTH"),
      rb_str_new2(LAUNCH_JOBKEY_CAL_MONTH));

  rb_const_set(mLaunchJobKey, rb_intern("RESOURCELIMIT_CORE"),
      rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_CORE));
  rb_const_set(mLaunchJobKey, rb_intern("RESOURCELIMIT_CPU"),
      rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_CPU));
  rb_const_set(mLaunchJobKey, rb_intern("RESOURCELIMIT_DATA"),
      rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_DATA));
  rb_const_set(mLaunchJobKey, rb_intern("RESOURCELIMIT_FSIZE"),
      rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_FSIZE));
  rb_const_set(mLaunchJobKey, rb_intern("RESOURCELIMIT_MEMLOCK"),
      rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_MEMLOCK));
  rb_const_set(mLaunchJobKey, rb_intern("RESOURCELIMIT_NOFILE"),
      rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_NOFILE));
  rb_const_set(mLaunchJobKey, rb_intern("RESOURCELIMIT_NPROC"),
      rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_NPROC));
  rb_const_set(mLaunchJobKey, rb_intern("RESOURCELIMIT_RSS"),
      rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_RSS));
  rb_const_set(mLaunchJobKey, rb_intern("RESOURCELIMIT_STACK"),
      rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_STACK));

  rb_const_set(mLaunchJobKey, rb_intern("DISABLED_MACHINETYPE"),
      rb_str_new2(LAUNCH_JOBKEY_DISABLED_MACHINETYPE));
  rb_const_set(mLaunchJobKey, rb_intern("DISABLED_MODELNAME"),
      rb_str_new2(LAUNCH_JOBKEY_DISABLED_MODELNAME));

  /*
   * Launch::Socket holds various socket keys
   */
  mLaunchSocket = rb_define_module_under(mLaunch, "Socket");
  rb_const_set(mLaunchSocket, rb_intern("TYPE"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_TYPE));
  rb_const_set(mLaunchSocket, rb_intern("PASSIVE"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_PASSIVE));
  rb_const_set(mLaunchSocket, rb_intern("BONJOUR"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_BONJOUR));
  rb_const_set(mLaunchSocket, rb_intern("SECUREWITHKEY"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_SECUREWITHKEY));
  rb_const_set(mLaunchSocket, rb_intern("PATHNAME"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_PATHNAME));
  rb_const_set(mLaunchSocket, rb_intern("PATHMODE"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_PATHMODE));
  rb_const_set(mLaunchSocket, rb_intern("NODENAME"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_NODENAME));
  rb_const_set(mLaunchSocket, rb_intern("SERVICENAME"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_SERVICENAME));
  rb_const_set(mLaunchSocket, rb_intern("FAMILY"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_FAMILY));
  rb_const_set(mLaunchSocket, rb_intern("PROTOCOL"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_PROTOCOL));
  rb_const_set(mLaunchSocket, rb_intern("MULTICASTGROUP"),
      rb_str_new2(LAUNCH_JOBSOCKETKEY_MULTICASTGROUP));

  /*
   * Launch::JobPolicy holds the job policy keys
   */
  mLaunchJobPolicy = rb_define_module_under(mLaunch, "JobPolicy");
  rb_const_set(mLaunchJobPolicy, rb_intern("DENYCREATINGOTHERJOBS"),
      rb_str_new2(LAUNCH_JOBPOLICY_DENYCREATINGOTHERJOBS));
}
