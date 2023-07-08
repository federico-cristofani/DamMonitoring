package it.unipi.iot.damonitoring.controller.user.commands;

import it.unipi.iot.damonitoring.controller.control.ControlLogic;
import it.unipi.iot.damonitoring.controller.user.commands.exceptions.CommandException;
import picocli.CommandLine.*;

@Command(
        name = "Threshold",
        description = "Threshold levels configuration.",
        mixinStandardHelpOptions = true,
        version = "Controller 1.0"
)
public class Threshold implements BaseCommand {
    @Parameters(index = "0", description = "Threshold name.", arity = "0,1")
    String threshold;

    @Parameters(index = "1", description = "New threshold level specified in meters.", arity = "0,1")
    Float value;

    @Override
    public Integer call() throws Exception {

        if(threshold == null){
            System.out.printf("Threshold \"MAX\" = %.2f [m]%n",
                    ControlLogic.getMaxThreshold());
            System.out.printf("Threshold \"MIN\" = %.2f [m]%n",
                    ControlLogic.getMinThreshold());
            System.out.printf("Threshold \"SAFE\" = %.2f [m]%n",
                    ControlLogic.getSafeThreshold());
        }
        else if(value == null){
            float thresholdValue;
            switch (threshold.toLowerCase()) {
                case "max":
                    thresholdValue = ControlLogic.getMaxThreshold();
                    break;
                case "min":
                    thresholdValue = ControlLogic.getMinThreshold();
                    break;
                case "safe":
                    thresholdValue = ControlLogic.getSafeThreshold();
                    break;
                default:
                    throw new CommandException("Threshold not available");
            }
            System.out.printf("Threshold \"%s\" = %.2f [m]%n", threshold.toUpperCase(), thresholdValue);
        }
        else {
            switch (threshold.toLowerCase()) {
                case "max":
                    ControlLogic.setMaxThreshold(value);
                    break;
                case "min":
                    ControlLogic.setMinThreshold(value);
                    break;
                case "safe":
                    ControlLogic.setSafeThreshold(value);
                    break;
                default:
                    throw new CommandException("Threshold not available");
            }
            System.out.printf("New \"%s\" threshold set to %.2f [m]%n", threshold.toUpperCase(), value);
        }
        return 0;
    }
}
