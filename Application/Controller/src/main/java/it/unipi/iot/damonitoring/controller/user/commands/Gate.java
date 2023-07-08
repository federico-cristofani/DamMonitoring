package it.unipi.iot.damonitoring.controller.user.commands;

import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.controller.coap.CoapManager;
import it.unipi.iot.damonitoring.controller.coap.exceptions.CoapException;
import it.unipi.iot.damonitoring.controller.control.ControlLogic;
import it.unipi.iot.damonitoring.controller.control.enumerates.OperativeMode;
import it.unipi.iot.damonitoring.controller.user.commands.exceptions.CommandException;
import it.unipi.iot.damonitoring.entities.Resource;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;
import picocli.CommandLine.*;

import java.util.stream.Collectors;

@Command(
        name = "Gate",
        description = "Control the gates.",
        mixinStandardHelpOptions = true,
        version = "Controller 1.0"
)
public class Gate implements BaseCommand{

    @Option(
            names = {"-l", "--list"},
            description = "Retrieve list of available gates."
    )
    Boolean list = false;

    @Parameters(index = "0", description = "Gate name.", arity = "0,1")
    private String gateName;

    @Parameters(index = "1", description = "Open the gate to the specified level (0-100).", arity = "0,1")
    private Integer openingLevel;

    @Override
    public Integer call() throws Exception {

        if(list){
            list();
        }
        else if(gateName == null){
            for(Resource gate: DataManager.getInstance().gateResources()){
                getLevel(gate.getName());
            }
        } else if (openingLevel == null) {
            getLevel(gateName);
        } else {
            setLevel(gateName, openingLevel);
        }
        return 0;
    }

    private void list() throws PersistenceException {
        System.out.println("Online gates: \n" + DataManager.getInstance()
                .gateResources().stream()
                .map(res -> String.format("* %s [%s]\t%s", res.getName().toUpperCase(), res.getTag(), res.getDescription()))
                .collect(Collectors.joining("\n")));
    }

    private void setLevel(String gateName, Integer level) throws CoapException, CommandException, PersistenceException {
        if(ControlLogic.getRunningMode().equals(OperativeMode.AUTO)){
            throw new CommandException("Set MANUAL mode to control the gates");
        }
        ControlLogic.setGateOpening(gateName, level);
        System.out.printf("Gate \"%s\" opening level = %d%%%n", gateName.toUpperCase(), level);
    }

    private void getLevel(String gateName) throws CoapException {
        System.out.printf("Gate \"%s\" opening level = %d%%%n",
                gateName.toUpperCase(), CoapManager.getGateOpening(gateName));
    }
}
