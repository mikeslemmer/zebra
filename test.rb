require 'socket'

hostname = 'localhost'
port = 9090

def check_result(sock, expected)
  line = sock.gets
  if line.chomp != expected
    puts "Expected: " + expected + "; Got: " + line
    raise "failed"
  end
end


sock = TCPSocket.open(hostname, port)

sock.puts "blah"
check_result(sock, "(error) ERR unknown command 'blah'")

sock.puts "zadd test 1 first 2 second 3 third"
check_result(sock, "(integer) 3")

sock.close


