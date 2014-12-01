JAR=jar
JARFLAGS=cvmf
JAVAC=javac
JAVACFLAGS=-cp src:twitter4j-core.jar:twitter4j-stream.jar

collector.jar:	MANIFEST.MF src/collector.java twitter4j-core.jar \
		twitter4j-stream.jar
	$(JAVAC) $(JAVACFLAGS)	src/collector.java src/twitter4j/Status.java \
				src/twitter4j/StatusJSONImpl.java
	$(JAR) $(JARFLAGS)	MANIFEST.MF collector.jar twitter4j-core.jar \
				twitter4j-stream.jar \
				-C src collector.class \
				-C src collector\$$1.class \
				-C src twitter4j/Status.class \
				-C src twitter4j/StatusJSONImpl.class

.PHONY: clean

clean:
	rm -fv src/collector.class src/collector\$$1.class \
	src/twitter4j/Status.class src/twitter4j/StatusJSONImpl.class \
	collector.jar
