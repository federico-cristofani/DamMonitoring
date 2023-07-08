package it.unipi.iot.damonitoring.controller.user.commands;
import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.entities.FlowRate;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;
import picocli.CommandLine.*;

import java.text.SimpleDateFormat;
import java.util.Arrays;

@Command(
        name = "Flow",
        mixinStandardHelpOptions = true,
        description = "Show the current flow rates.",
        version = "Controller 1.0"
)
public class Flow implements BaseCommand {

    private static final String[] CHANNELS = {"INFLOW", "OUTFLOW-1", "OUTFLOW-2"};

    private static final SimpleDateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");

    @Option(names = {"-l", "--list"}, description = "Retrieve list of available channels.")
    Boolean list = false;

    @Parameters(index="0", description = "Channel identifier.", arity = "0,1")
    String channel;

    @Override
    public Integer call() throws Exception {
        if(list){
            list();
        }
        else if(channel == null){
            for(String channel: CHANNELS){
                lastRecord(channel);
            }
        }
        else {
            lastRecord(channel);
        }
        return 0;
    }

    private void list(){
        Arrays.stream(CHANNELS).forEach(c -> System.out.printf("* %s%n", c));
        //System.out.println("Available channels: \n * " + String.join("\n * ", CHANNELS));
    }

    private void lastRecord(String channel) throws PersistenceException {
        FlowRate flowRate = DataManager.getInstance().retrieveFlowRate(channel);

        if(flowRate == null){
            System.out.printf("Channel \"%s\": no available data%n",
                    channel.toUpperCase());
            return;
        }
        System.out.printf("Flow rate at %s in channel \"%s\" = %.2f [m^3/s]%n",
                formatter.format(flowRate.getTimestamp()), channel.toUpperCase(), flowRate.getValue());
    }
}
