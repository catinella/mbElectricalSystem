#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <mbesUtilities.h>

int main() {

	logMsg("This is a log-message (pid=%d): \"%s\"\n", getpid(), "You should see this message");

	return(0);
}
