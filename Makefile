INSTALLATION_DIRECTORY = "/opt/Rasprinter"

install:
	@printf "\033[33mBuilding software...\033[0m\n"
	make -C Software
	
	@printf "\033[33mInstalling software to /opt...\033[0m\n"
	mkdir -p $(INSTALLATION_DIRECTORY)
	cp Software/Rasprinter $(INSTALLATION_DIRECTORY)
	cp Software/Picture.bmp $(INSTALLATION_DIRECTORY)
	cp Software/Sansation_Regular.ttf $(INSTALLATION_DIRECTORY)
	
	@printf "\033[33mInstalling init script...\033[0m\n"
	cp Resources/Rasprinter /etc/init.d
	update-rc.d Rasprinter defaults

uninstall:
	@printf "\033[33mRemoving software...\033[0m\n"
	rm -rf $(INSTALLATION_DIRECTORY)
	
	@printf "\033[33mRemoving init script...\033[0m\n"
	update-rc.d Rasprinter disable
	rm -f /etc/init.d/Rasprinter
