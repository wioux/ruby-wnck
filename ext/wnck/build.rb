#!/usr/bin/ruby

require 'erb'

class String
  def uscore
    gsub(/([a-z])?([A-Z])/){ |x,y| "#{$1 && $1+'_'}#{$2.downcase}" }
  end
end

module Code
  DIR = File.dirname(File.expand_path(__FILE__))
  PROTOS = File.open("#{DIR}/prototypes.include", 'w')
  DEFS = File.open("#{DIR}/definitions.include", 'w')
  INITS = File.open("#{DIR}/initializations.include", 'w')
  SIGNAL_SWITCH = File.open("#{DIR}/signals.include", 'w')
  SIGNAL_TYPES = []
  def from_template(template, domain, method, opts={})
    opts[:cprefix] ||= 'get_'
    opts[:selector] ||= "#{method}"
    opts[:rwnck_name] ||= "#{opts[:selector]}".gsub('?', '_p')
    domain = "#{domain}"
    method = "#{method}"
    vdomain = "vWnck#{domain}"
    codomain = "#{opts[:codomain]}"
    vcodomain = "vWnck#{codomain}"
    libmethod = "wnck_#{domain.uscore}_#{opts[:cprefix]}#{method}"
    rbmethod = "rwnck_#{domain.uscore}_#{opts[:rwnck_name]}"
    INITS.puts("rb_define_method(#{vdomain}, \"#{opts[:selector]}\", #{rbmethod}, 0);")
    PROTOS.puts("VALUE #{rbmethod}(VALUE self);")
    DEFS.puts(ERB.new(File.read("#{DIR}/#{template}")).result(binding))
  end
  def query_int(domain, method, opts={})
    from_template('query_int.c.erb', domain, method, opts)
  end
  def query_str(domain, method, opts={})
    from_template('query_str.c.erb', domain, method, opts)
  end
  def query_bool(domain, method, opts={})
    opts = opts.merge({:selector => "#{method}?"})
    from_template('query_bool.c.erb', domain, method, opts)
  end
  def delegate(domain, method, opts={})
    from_template('delegate.c.erb', domain, method, opts)
  end
  def query_assoc(domain, codomain, method, opts={})
    from_template('query_assoc.c.erb', domain, method,
                  opts.merge(:codomain => codomain))
  end
  def query_assocs(domain, codomain, method, opts={})
    from_template('query_assocs.c.erb', domain, method,
                  opts.merge(:codomain => codomain))
  end
  def signal_handler(*args)
    rbmethod = "wnck_callback_#{args.map(&:downcase).join('_')}"
    source = ERB.new(File.read("#{DIR}/handler.c.erb")).result(binding).strip
    unless SIGNAL_TYPES.include?(args)
      DEFS.puts(source)
      PROTOS.puts(source.lines.first.strip.chop+';')
      SIGNAL_TYPES << args
    end
    rbmethod
  end
  def signal(domain, signal, *args)
    handler = signal_handler(*args)
    SIGNAL_SWITCH.puts <<-EOF
    if (klass == vWnck#{domain} && !strcmp(signal, "#{signal}"))
      return (GCallback)#{handler};
EOF
  end
end


include Code
query_assoc(:Screen, :Window, :active_window)
query_assoc(:Screen, :Window, :previously_active_window)
query_assocs(:Screen, :Window, :windows)
query_assocs(:Screen, :Window, :windows_stacked)
query_assoc(:Screen, :Workspace, :active_workspace)
query_assocs(:Screen, :Workspace, :workspaces)
# query_assoc_by_id(:Screen, :Workspace, :workspace)
query_int(:Screen, :workspace_count)
# workspace layouts...
query_int(:Screen, :number)
query_int(:Screen, :width)
query_int(:Screen, :height)
query_str(:Screen, :window_manager_name)
query_int(:Screen, :background_pixmap)
query_bool(:Screen, :showing_desktop)
delegate(:Screen, :force_update)
signal(:Screen, 'active-window-changed', :Screen, :Window,)
signal(:Screen, 'active-workspace-changed', :Screen, :Workspace)
signal(:Screen, 'application-closed', :Screen, :Application)
signal(:Screen, 'application-opened', :Screen, :Application)
signal(:Screen, 'background-changed', :Screen)
signal(:Screen, 'class-group-closed', :Screen, :ClassGroup)
signal(:Screen, 'class-group-opened', :Screen, :ClassGroup)
signal(:Screen, 'showing-desktop-changed', :Screen)
signal(:Screen, 'viewports-changed', :Screen)
signal(:Screen, 'window-manager-changed', :Screen)
signal(:Screen, 'window-opened', :Screen, :Window)
signal(:Screen, 'window-stacking-changed', :Screen)
signal(:Screen, 'workspace-created', :Screen, :Workspace)
signal(:Screen, 'workspace-destroyed', :Screen, :Workspace)

query_assoc(:Window, :Screen, :screen)
query_assoc(:Window, :ClassGroup, :class_group)
query_assoc(:Window, :Application, :application)
query_bool(:Window, :has_name, :cprefix => '')
query_str(:Window, :name)
query_bool(:Window, :has_icon_name, :cprefix => '')
query_str(:Window, :icon_name)
query_bool(:Window, :icon_is_fallback, :cprefix => '')
query_int(:Window, :group_leader)
query_int(:Window, :xid)
# query_str(:Window, :class_group_name)
# query_str(:Window, :class_instance_name)
query_str(:Window, :session_id)
query_str(:Window, :session_id_utf8)
query_int(:Window, :pid)
query_int(:Window, :type, :cprefix => 'get_window_')
query_int(:Window, :state)

query_bool(:Window, :minimized, :cprefix => 'is_')
query_bool(:Window, :maximized_horizontally, :cprefix => 'is_')
query_bool(:Window, :maximized_vertically, :cprefix => 'is_')
query_bool(:Window, :maximized, :cprefix => 'is_')
query_bool(:Window, :shaded, :cprefix => 'is_')
query_bool(:Window, :pinned, :cprefix => 'is_')
query_bool(:Window, :sticky, :cprefix => 'is_')
query_bool(:Window, :above, :cprefix => 'is_')
query_bool(:Window, :below, :cprefix => 'is_')
query_bool(:Window, :skip_pager, :cprefix => 'is_')
query_bool(:Window, :skip_tasklist, :cprefix => 'is_')
query_bool(:Window, :fullscreen, :cprefix => 'is_')
query_bool(:Window, :needs_attention, :cprefix => '')
query_bool(:Window, :or_transient_needs_attention, :cprefix => '')

query_int(:Window, :actions)
delegate(:Window, :minimize)
# delegate(:Window, :unminimize) requires timestamp as 2nd arg
delegate(:Window, :maximize_horizontally)
delegate(:Window, :unmaximize_horizontally)
delegate(:Window, :maximize_vertically)
delegate(:Window, :unmaximize_vertically)
delegate(:Window, :maximize)
delegate(:Window, :unmaximize)
delegate(:Window, :shade)
delegate(:Window, :unshade)
delegate(:Window, :pin)
delegate(:Window, :unpin)
delegate(:Window, :stick)
delegate(:Window, :unstick)
delegate(:Window, :make_above)
delegate(:Window, :unmake_above)
delegate(:Window, :make_below)
delegate(:Window, :unmake_below)

query_bool(:Window, :active, :cprefix => 'is_')
query_bool(:Window, :most_recently_activated, :cprefix => 'is_')

delegate(:Window, :keyboard_move)
delegate(:Window, :keyboard_size)

# signal(:Window, 'actions-changed', :Window, :int, :int)
signal(:Window, 'class-changed', :Window)
signal(:Window, 'geometry-changed', :Window)
signal(:Window, 'icon-changed', :Window)
signal(:Window, 'name-changed', :Window)
signal(:Window, 'role-changed', :Window)
# signal(:Window, 'state-changed', :Window, :int, :int)

# query_str(:ClassGroup, :id) not implemented til libwnck 3.2
query_assocs(:ClassGroup, :Window, :windows)
query_str(:ClassGroup, :name)
query_str(:ClassGroup, :res_class)
signal(:ClassGroup, 'icon-changed', :ClassGroup)
signal(:ClassGroup, 'name-changed', :ClassGroup)

query_assoc(:Workspace, :Screen, :screen)
query_int(:Workspace, :number)
query_str(:Workspace, :name)
query_int(:Workspace, :width)
query_int(:Workspace, :height)
query_int(:Workspace, :viewport_x)
query_int(:Workspace, :viewport_y)
query_bool(:Workspace, :virtual, :cprefix => 'is_')
query_int(:Workspace, :layout_row)
query_int(:Workspace, :layout_column)
signal(:Workspace, 'name-changed', :Workspace)

query_int(:Application, :xid)
query_str(:Application, :name)
query_str(:Application, :icon_name)
query_int(:Application, :pid)
query_bool(:Application, :icon_is_fallback)
query_str(:Application, :startup_id)
query_int(:Application, :n_windows)
signal(:Application, 'icon-changed', :Application)
signal(:Application, 'name-changed', :Application)
