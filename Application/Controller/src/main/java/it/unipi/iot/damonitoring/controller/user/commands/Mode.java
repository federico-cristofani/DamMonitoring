package it.unipi.iot.damonitoring.controller.user.commands;

import it.unipi.iot.damonitoring.controller.control.ControlLogic;
import it.unipi.iot.damonitoring.controller.control.enumerates.OperativeMode;
import it.unipi.iot.damonitoring.controller.control.exceptions.ControlException;
import picocli.CommandLine.*;

@Command(
        name = "Mode",
        description = "Mode configuration.",
        mixinStandardHelpOptions = true,
        version = "Controller 1.0"
)
public class Mode implements BaseCommand {

    @Parameters(index = "0", description = "Set mode [AUTO/MANUAL].", arity = "0,1")
    private String mode;

    @Override
    public Integer call() {
        if(mode == null){
            System.out.println("Current mode: " + ControlLogic.getRunningMode().toString()
                    .toUpperCase());
        }
        else {
            try {
                OperativeMode newMode;
                switch (mode.toUpperCase()){
                    case "AUTO":
                        newMode = ControlLogic.startAutomaticControlUnit();
                        break;
                    case "MANUAL":
                        newMode = ControlLogic.stopAutomaticControlUnit();
                        break;
                    default:
                        System.out.println("Available modes: AUTO/MANUAL");
                        return 0;
                }
                System.out.println("Configured mode: " + newMode);
            }
            catch (ControlException ex){
                System.out.println(ex.getMessage());
            }
        }
        return 0;
    }
}
