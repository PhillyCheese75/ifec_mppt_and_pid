################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/bin/cl2000" -v28 -ml -mt --cla_support=cla1 --float_support=fpu32 --tmu_support=tmu0 --vcu_support=vcu2 -Ooff --include_path="C:/Users/phill/workspace_v9_2/IFECNEW" --include_path="C:/Users/phill/workspace_v9_2/IFECNEW/include" --include_path="C:/Users/phill/workspace_v9_2/IFECNEW/src" --include_path="C:/Users/phill/workspace_v9_2/IFECNEW" --include_path="C:/Users/phill/workspace_v9_2/IFECNEW/device" --include_path="C:/ti/C2000Ware_3_02_00_00_Software/driverlib/f2837xd/driverlib" --include_path="C:/ti/ccs920/ccs/tools/compiler/ti-cgt-c2000_18.12.3.LTS/include" --advice:performance=all --define=_FLASH --define=CPU1 --diag_suppress=10063 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


