gtk_flags=`pkg-config --cflags gtk+-3.0`
gtk_libs=`pkg-config --libs gtk+-3.0`

gtk:
	gcc ${gtk_flags} -o ${MAIN} ${MAIN}.cpp ${gtk_libs}

clean:
	rm -f ${MAIN}
