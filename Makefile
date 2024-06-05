# Directories
OBJDIR = obj
BINDIR = bin

# Create directories if they don't exist
$(shell mkdir -p $(OBJDIR) $(BINDIR))

# Targets
all: $(BINDIR)/server $(BINDIR)/client

$(BINDIR)/server: $(OBJDIR)/server.o
	gcc -Wall -o $(BINDIR)/server $(OBJDIR)/server.o

$(BINDIR)/client: $(OBJDIR)/client.o
	gcc -Wall -o $(BINDIR)/client $(OBJDIR)/client.o

$(OBJDIR)/server.o: server.c
	gcc -Wall -c server.c -o $(OBJDIR)/server.o

$(OBJDIR)/client.o: client.c
	gcc -Wall -c client.c -o $(OBJDIR)/client.o

clean:
	rm -f $(OBJDIR)/*.o $(BINDIR)/server $(BINDIR)/client
