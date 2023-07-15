package it.unipi.iot.damonitoring.controller.user.commands;

import picocli.CommandLine.Command;

@Command(
        name = "Controller",
        version = "Controller 1.0",
        subcommands = {
            Mode.class,
            Threshold.class,
            Flow.class,
            Level.class,
            Alarm.class,
            Gate.class,
            Commands.Shutdown.class,
            Commands.Help.class
        }
)
public final class Commands {

    @Command(
            name = "Help",
            description = "Show available commands.",
            mixinStandardHelpOptions = true,
            version = "Controller 1.0"
    )
    protected static class Help implements Runnable{
        @Override
        public void run() { /*Placeholder command */ }
    }

    @Command(
            name = "Shutdown",
            description = "Shutdown the application.",
            mixinStandardHelpOptions = true,
            version = "Controller 1.0"
    )
    protected static class Shutdown implements BaseCommand {
        @Override
        public Integer call() { System.exit(0); return 0;}
    }
}

