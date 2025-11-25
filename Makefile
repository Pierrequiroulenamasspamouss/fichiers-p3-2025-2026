SRCS1 = main_features.c BTree.c Dict.c HierarchicalClustering.c LinkedList.c
SRCS2 = main_phylo.c BTree.c Dict.c HierarchicalClustering.c LinkedList.c Phylogenetic.c
OBJS1 = $(SRCS1:%.c=%.o)
OBJS2 = $(SRCS2:%.c=%.o)
TARGET1 = hcfeatures
TARGET2 = hcphylo
CC = gcc
CFLAGS = -std=c99 --pedantic -Wall -Wextra -Wmissing-prototypes -g3
LDFLAGS = -lm

.PHONY: all clean run

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJS1)
	$(CC) -o $@ $^ $(LDFLAGS)

$(TARGET2): $(OBJS2)
	$(CC) -o $@ $^ $(LDFLAGS)

run : $(TARGET1)
	./$(TARGET1) -k 4 Data/features-zoo.csv 

run-phylo: $(TARGET2)
	./$(TARGET2) Data/dna-virus.csv Data/dna-virus.newick

clean:
	rm -f $(OBJS1) $(OBJS2) $(TARGET1) $(TARGET2)

BTree.o: BTree.c BTree.h
Dict.o: Dict.c Dict.h
HierarchicalClustering.o: HierarchicalClustering.c Dict.h \
  HierarchicalClustering.h LinkedList.h BTree.h
LinkedList.o: LinkedList.c LinkedList.h
Phylogenetic.o: Phylogenetic.c LinkedList.h Dict.h Phylogenetic.h \
  HierarchicalClustering.h BTree.h
main_features.o: main_features.c Dict.h LinkedList.h BTree.h \
  HierarchicalClustering.h
main_phylo.o: main_phylo.c Dict.h LinkedList.h BTree.h Phylogenetic.h \
  HierarchicalClustering.h
