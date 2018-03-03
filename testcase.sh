#! /bin/bash
HOSTNAME=$1
echo -e "GET http://people.duke.edu/~tkb13/courses/ece650/resources/awesome.txt HTTP/1.1\r\nHost: people.duke.edu\r\nCache-Control: no-cache, no-store\r\n\r\n" | nc $1 12345 &

echo -e "GET http://people.duke.edu/~bmr23/ece650/class.html HTTP/1.1\r\nHost: people.duke.edu\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:58.0) Gecko/20100101 Firefox/58.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nCache-Control: max-age=0\r\n\r\n" | nc $1 12345 &

echo -e "GET http://people.duke.edu/~tkb13/courses/ece650/resources/awesome.txt HTTP/1.1\r\nHost: people.duke.edu\r\n\r\n" | nc $1 12345 &

sleep 5

echo -e "GET http://people.duke.edu/~tkb13/courses/ece650/resources/awesome.txt HTTP/1.1\r\nHost: people.duke.edu\r\nCache-Control: must-revalidate\r\n\r\n" | nc $1 12345 &

sleep 5

echo -e "GET http://people.duke.edu/~tkb13/courses/ece650/resources/awesome.txt HTTP/1.1\r\nHost: people.duke.edu\r\nCache-Control: must-revalidate\r\n\r\n" | nc $1 12345 &
sleep 5

echo -e "GET http://people.duke.edu/~tkb13/courses/ece650/resources/awesome.txt HTTP/1.1\r\nHost: people.duke.edu\r\nCache-Control: must-revalidate\r\n\r\n" | nc $1 12345 &
sleep 5

echo -e "GET http://people.duke.edu/~tkb13/courses/ece650/resources/awesome.txt HTTP/1.1\r\nHost: people.duke.edu\r\nCache-Control: must-revalidate\r\n\r\n" | nc $1 12345 &