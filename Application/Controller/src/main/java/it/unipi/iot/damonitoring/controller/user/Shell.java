package it.unipi.iot.damonitoring.controller.user;

import it.unipi.iot.damonitoring.controller.control.ControlLogic;
import it.unipi.iot.damonitoring.controller.user.commands.Commands;

import picocli.CommandLine;
import picocli.CommandLine.UnmatchedArgumentException;
import picocli.CommandLine.ParameterException;

import java.io.*;
import java.util.*;
import java.util.stream.Collectors;

public class Shell {
    private static final String BANNER_FILE = "/banner.txt";
    private static final Shell instance;
    private static final String banner;
    private final CommandLine commandLine;

    static {

        // Eagle initialization
        instance = new Shell();

        // Load banner
        try(InputStream resource = Shell.class.getResourceAsStream(BANNER_FILE)){
            banner = new BufferedReader(
                    new InputStreamReader(Objects.requireNonNull(resource)))
                    .lines()
                    .collect(Collectors.joining("\n"));
        }catch (IOException ex){
            System.out.println("Error during initialization: " + ex.getMessage());
            throw new RuntimeException(ex);
        }
    }

    public static Shell getInstance(){
        return instance;
    }

    private Shell(){
        Commands commands = new Commands();

        // Configure command line interface
        commandLine = new CommandLine(commands);
        commandLine.setSubcommandsCaseInsensitive(true);
        commandLine.setOut(new PrintWriter(System.out, true));
        commandLine.setErr(new PrintWriter(System.out, true));
        commandLine.setExecutionExceptionHandler((exception, commandLine, parseResult) -> {
            System.out.println(exception.getMessage());
            return 0;
        });
    }

    public void repl(){
        try {
            Scanner sc = new Scanner(System.in);

            // Show banner
            System.out.println(banner);

            // REPL
            String command;
            System.out.print(buildPrompt());
            while ((command = sc.nextLine()) != null) {
                executeCommand(command);
                System.out.print(buildPrompt());
            }
        } catch (RuntimeException ex) {
            System.out.println("Unexpected error: " + ex.getMessage());
            System.exit(-1);
        }
    }

    private String buildPrompt() {
        return String.format("Controller [%s] > ", ControlLogic.getRunningMode());
    }

    private void executeCommand(String command){
        try {
            if(command.trim().equalsIgnoreCase("help")){
                System.out.println(commandLine.getUsageMessage());
            }
            else if(!command.trim().equals("")){
                String[] args = command.split(" ");

                int resCode = commandLine.execute(commandLine
                    .parseArgs(args)
                    .expandedArgs()
                    .toArray(new String[0]));

                if(resCode < 0){
                    System.out.println("Something went wrong");
                }
            }
        }
        catch (UnmatchedArgumentException ex){
            if(ex.getMessage().contains("index")){
                System.out.println("Unknown command");
                commandLine.execute("help");
            }
            else{
                System.out.println(ex.getMessage());
            }
        } catch (ParameterException ex){
            System.out.println(ex.getMessage());
        }
    }
}
