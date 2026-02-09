
export ICO2CODE := $(TOOLS_BINDIR)/Ico2Ccode

$(ICO2CODE):
	dotnet publish $(TOOLS_DIR)/Ico2Ccode/Ico2Ccode.sln -r linux-x64 --self-contained false --property:PublishDir=$(TOOLS_BINDIR)