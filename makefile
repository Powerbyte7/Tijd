# ----------------------------
# Makefile Options
# ----------------------------

NAME = TIJD
ICON = icon.png
DESCRIPTION = "A game with idle progression"
COMPRESSED = NO
ARCHIVED = NO
HAS_PRINTF = NO

CFLAGS = -Wall -Wextra -Oz -fshort-enums
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
