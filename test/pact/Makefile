SHELL := bash
# standard colors
BLACK        := $(shell tput -Txterm setaf 0)
RED          := $(shell tput -Txterm setaf 1)
GREEN        := $(shell tput -Txterm setaf 2)
YELLOW       := $(shell tput -Txterm setaf 3)
LIGHTPURPLE  := $(shell tput -Txterm setaf 4)
PURPLE       := $(shell tput -Txterm setaf 5)
BLUE         := $(shell tput -Txterm setaf 6)
WHITE        := $(shell tput -Txterm setaf 7)

.PHONY: all build test_contracts

all:
	@echo ${GREEN}Available commands${YELLOW}
	@echo "build 			${WHITE}Install dependencies and build tests${YELLOW}"
	@echo "test_contracts 		${WHITE}Runs tests${YELLOW}"

build:
	@mkdir -p build && cd build && \
	conan install .. --build missing && \
	cmake .. && cmake --build .

test_contracts:
	@cd build; ./bin/contracts
