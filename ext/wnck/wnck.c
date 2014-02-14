#include <ruby.h>

// requires libwnck-dev
#define WNCK_I_KNOW_THIS_IS_UNSTABLE
#include <libwnck/libwnck.h>
#include <dlfcn.h>

typedef struct WnckObject {
  void *ptr;
  VALUE children;
} WnckObject;

VALUE vWnck = Qnil;
VALUE vWnckObject = Qnil;
VALUE vWnckScreen = Qnil;
VALUE vWnckWorkspace = Qnil;
VALUE vWnckWindow = Qnil;
VALUE vWnckClassGroup = Qnil;
VALUE vWnckApplication = Qnil;

void Init_wnck(void);
void rwnck_object_mark(void *ptr);
void rwnck_object_free(void *ptr);

VALUE rwnck_object_signal_connect(VALUE self, VALUE signal);

VALUE rwnck_screen_default(VALUE self);
VALUE rwnck_screen_get(VALUE self, VALUE i);
VALUE rwnck_screen_move_viewport(VALUE self, VALUE x, VALUE y);
VALUE rwnck_screen_showing_desktop_e(VALUE self, VALUE mode);
VALUE rwnck_window_get(VALUE self, VALUE i);
VALUE rwnck_class_group_get(VALUE self, VALUE id);

#include "prototypes.include"

void Init_wnck(void) {
  int argc = 0;
  char **argv = NULL;
  gdk_init(&argc, &argv);
  
  vWnck = rb_define_module("Wnck");
  vWnckObject= rb_define_class_under(vWnck, "Object", rb_cObject);
  vWnckScreen= rb_define_class_under(vWnck, "Screen", vWnckObject);
  vWnckWorkspace= rb_define_class_under(vWnck, "Workspace", vWnckObject);
  vWnckApplication= rb_define_class_under(vWnck, "Application", vWnckObject);
  vWnckWindow = rb_define_class_under(vWnck, "Window", vWnckObject);
  vWnckClassGroup = rb_define_class_under(vWnck, "ClassGroup", vWnckObject);

  rb_define_const(vWnckWindow, "TYPE_NORMAL", INT2FIX(0));
  rb_define_const(vWnckWindow, "TYPE_DESKTOP", INT2FIX(1));
  rb_define_const(vWnckWindow, "TYPE_DOCK", INT2FIX(2));
  rb_define_const(vWnckWindow, "TYPE_DIALOG", INT2FIX(3));
  rb_define_const(vWnckWindow, "TYPE_TOOLBAR", INT2FIX(4));
  rb_define_const(vWnckWindow, "TYPE_MENU", INT2FIX(5));
  rb_define_const(vWnckWindow, "TYPE_UTILITY", INT2FIX(6));
  rb_define_const(vWnckWindow, "TYPE_SPLASHSCREEN", INT2FIX(7));

  rb_define_const(vWnckWindow, "STATE_MINIMIZED", INT2FIX(1 << 0));
  rb_define_const(vWnckWindow, "STATE_MAXIMIZED_HORITONTALLY", INT2FIX(1 << 1));
  rb_define_const(vWnckWindow, "STATE_MAXIMIZED_VERTICALLY", INT2FIX(1 << 2));
  rb_define_const(vWnckWindow, "STATE_SHADED", INT2FIX(1 << 3));
  rb_define_const(vWnckWindow, "STATE_SKIP_PAGER", INT2FIX(1 << 4));
  rb_define_const(vWnckWindow, "STATE_SKIP_TASKLIST", INT2FIX(1 << 5));
  rb_define_const(vWnckWindow, "STATE_STICKY", INT2FIX(1 << 6));
  rb_define_const(vWnckWindow, "STATE_HIDDEN", INT2FIX(1 << 7));
  rb_define_const(vWnckWindow, "STATE_FULLSCREEN", INT2FIX(1 << 8));
  rb_define_const(vWnckWindow, "STATE_DEMANDS_ATTENTION", INT2FIX(1 << 9));
  rb_define_const(vWnckWindow, "STATE_URGENT", INT2FIX(1 << 10));
  rb_define_const(vWnckWindow, "STATE_ABOVE", INT2FIX(1 << 11));
  rb_define_const(vWnckWindow, "STATE_BELOW", INT2FIX(1 << 12));

  rb_define_const(vWnckWindow, "ACTION_MOVE", INT2FIX(1 << 0));
  rb_define_const(vWnckWindow, "ACTION_RESIZE", INT2FIX(1 << 1));
  rb_define_const(vWnckWindow, "ACTION_SHADE", INT2FIX(1 << 2));
  rb_define_const(vWnckWindow, "ACTION_STICK", INT2FIX(1 << 3));
  rb_define_const(vWnckWindow, "ACTION_MAXIMIZE_HORIZONTALLY", INT2FIX(1 << 4));
  rb_define_const(vWnckWindow, "ACTION_MAXIMIZE_VERTICALLY", INT2FIX(1 << 5));
  rb_define_const(vWnckWindow, "ACTION_CHANGE_WORKSPACE", INT2FIX(1 << 6));
  rb_define_const(vWnckWindow, "ACTION_CLOSE", INT2FIX(1 << 7));
  rb_define_const(vWnckWindow, "ACTION_UNMAXIMIZE_HORIZONTALLY", INT2FIX(1 << 8));
  rb_define_const(vWnckWindow, "ACTION_UNMAXIMIZE_VERTICALLY", INT2FIX(1 << 9));
  rb_define_const(vWnckWindow, "ACTION_UNSHADE", INT2FIX(1 << 10));
  rb_define_const(vWnckWindow, "ACTION_UNSTICK", INT2FIX(1 << 11));
  rb_define_const(vWnckWindow, "ACTION_MINIMIZE", INT2FIX(1 << 12));
  rb_define_const(vWnckWindow, "ACTION_UNMINIMIZE", INT2FIX(1 << 13));
  rb_define_const(vWnckWindow, "ACTION_MAXIMIZE", INT2FIX(1 << 14));
  rb_define_const(vWnckWindow, "ACTION_UNMAXIMIZE", INT2FIX(1 << 15));
  rb_define_const(vWnckWindow, "ACTION_FULLSCREEN", INT2FIX(1 << 16));
  rb_define_const(vWnckWindow, "ACTION_ABOVE", INT2FIX(1 << 17));
  rb_define_const(vWnckWindow, "ACTION_BELOW", INT2FIX(1 << 18));
  
  rb_define_singleton_method(vWnckScreen, "default", rwnck_screen_default, 0);
  rb_define_singleton_method(vWnckScreen, "get", rwnck_screen_get, 1);
  rb_define_method(vWnckScreen, "move_viewport",
		   rwnck_screen_move_viewport, 2);
  rb_define_method(vWnckScreen, "showing_desktop=",
		   rwnck_screen_showing_desktop_p, 1);

  rb_define_singleton_method(vWnckWindow, "get", rwnck_window_get, 1);
  rb_define_method(vWnckWindow, "type=", rwnck_window_type, 1);

  rb_define_singleton_method(vWnckClassGroup, "get", rwnck_class_group_get, 1);

  rb_define_method(vWnckObject, "signal_connect",
		   rwnck_object_signal_connect, 1);
#include "initializations.include"
}

VALUE rwnck_object_create(VALUE klass, void *ptr)
{
  WnckObject *object;
  VALUE wrapped = Data_Make_Struct(klass, WnckObject,
				   rwnck_object_mark, rwnck_object_free,
				   object);
  object->ptr = ptr;
  object->children = rb_ary_new();
  return wrapped;
}
void rwnck_object_mark(void *ptr)
{
  rb_gc_mark(((struct WnckObject *)ptr)->children);
}
void rwnck_object_free(void *ptr)
{
  free(ptr);
}

typedef struct CallbackRequest {
  VALUE callback;
  VALUE args;
} CallbackRequest;
void *invoke_callback(void *_req) {
  CallbackRequest *req = _req;
  rb_proc_call(req->callback, req->args);
  return NULL;
}
void rwnck_callback_screen_window(WnckScreen *screen, WnckWindow *window,
				  gpointer *rb_proc) {
  VALUE rscreen, rwindow;
  VALUE rfunc = (VALUE)rb_proc;
  rscreen = rwnck_object_create(vWnckScreen, screen);
  rwindow = rwnck_object_create(vWnckWindow, window);

  VALUE args = rb_ary_new();
  rb_ary_push(args, rscreen);
  rb_ary_push(args, rwindow);

  CallbackRequest req;
  req.callback = rfunc;
  req.args = args;
  // rb_thread_call_with_gvl(invoke_callback, &req);
  rb_proc_call(rfunc, args);
}
GCallback rwnck_signal_func(VALUE klass, const char *signal) {
#include "signals.include"
  fprintf(stderr, "unknown signal: %s\n", signal);
  exit(1);
}
VALUE rwnck_object_signal_connect(VALUE self, VALUE signal)
{
  struct WnckObject *object;
  Data_Get_Struct(self, void, object);

  char *signalstr = StringValuePtr(signal);
  
  VALUE rfunc = rb_block_proc();
  VALUE klass = rb_funcall(self, rb_intern("class"), 0);
  GCallback cfunc = rwnck_signal_func(klass, signalstr);
  rb_global_variable(&rfunc);

  g_signal_connect(object->ptr, signalstr, cfunc, (gpointer)rfunc);

  return Qnil;
}


VALUE rwnck_screen_default(VALUE self) {
  WnckScreen *screen;
  screen = wnck_screen_get_default();
  if (!screen)
    return Qnil;
  wnck_screen_force_update(screen);
  return rwnck_object_create(vWnckScreen, screen);
}
VALUE rwnck_screen_get(VALUE self, VALUE i) {
  WnckScreen *screen;
  screen = wnck_screen_get(FIX2INT(i));
  if (!screen)
    return Qnil;
  wnck_screen_force_update(screen);
  return rwnck_object_create(vWnckScreen, screen);
}
VALUE rwnck_screen_move_viewport(VALUE self, VALUE x, VALUE y) {
  WnckScreen *screen;
  WnckObject *object;
  Data_Get_Struct(self, WnckObject, object);
  screen = object->ptr;
  wnck_screen_move_viewport(screen, FIX2INT(x), FIX2INT(y));
  return Qnil;
}
VALUE rwnck_screen_showing_desktop_e(VALUE self, VALUE mode) {
  WnckObject *object;
  WnckScreen *screen;
  Data_Get_Struct(self, WnckObject, object);
  screen = object->ptr;
  if (mode == Qnil || Qfalse)
    wnck_screen_toggle_showing_desktop(screen, FALSE);
  else
    wnck_screen_toggle_showing_desktop(screen, TRUE);
  return mode;
}


VALUE rwnck_window_get(VALUE self, VALUE i) {
  WnckWindow *window;
  window = wnck_window_get(FIX2INT(i));
  if (!window)
    return Qnil;
  return rwnck_object_create(vWnckWindow, window);
}
VALUE rwnck_window_type_e(VALUE self, VALUE type) {
  WnckObject *object;
  WnckWindow *window;
  Data_Get_Struct(self, WnckObject, object);
  window = object->ptr;
  wnck_window_set_window_type(window, (WnckWindowType)FIX2INT(type));
  return type;
}

VALUE rwnck_class_group_get(VALUE self, VALUE id) {
  WnckClassGroup *class_group;
  class_group = wnck_class_group_get(StringValuePtr(id));
  if (!class_group)
    return Qnil;
  return rwnck_objet_create(vWnckClassGroup, class_group);
}

#include "definitions.include"
