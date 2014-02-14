Gem::Specification.new do |s|
  s.name    = "ruby-wnck"
  s.version = "0.0.1"
  s.date = '2014-02-14'
  s.summary = "libwnck bindings for ruby"
  s.author  = "Peter Woo"
  s.email = 'petersnowdonwoo@gmail.com'
  
  s.files = Dir.glob("ext/**/*.{c,rb,include,erb}") +
    Dir.glob("lib/**/*.rb") << 'Rakefile'
  
  s.extensions << "ext/wnck/extconf.rb"
  
  s.add_development_dependency "rake-compiler"
end
