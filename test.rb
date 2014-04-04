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

# Start out by removing the test key.
sock.puts "del test"
sock.gets

sock.puts "blah"
check_result(sock, "(error) ERR unknown command 'blah'")

sock.puts "zadd test 1 first 2 second 3 third"
check_result(sock, "(integer) 3")

sock.puts "zrange 0 3"
check_result(sock, "1) \"first\"")


sock.close


