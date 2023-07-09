package it.unipi.iot.damonitoring.controller;

import it.unipi.iot.damonitoring.DataManager;
import it.unipi.iot.damonitoring.controller.control.AutomaticControlUnit;
import it.unipi.iot.damonitoring.controller.control.ControlLogic;
import it.unipi.iot.damonitoring.controller.control.exceptions.ControlException;
import it.unipi.iot.damonitoring.controller.user.Shell;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Controller {

	private static final Logger logger = LoggerFactory.getLogger(Controller.class);
	public static void main(String[] args) {

		// Set shutdown handler
		Runtime.getRuntime().addShutdownHook(new Thread(Controller::shutdown));

		// Eagle initialization
		try {
			DataManager.getInstance();
			AutomaticControlUnit.init();
		} catch (PersistenceException ex) {
			System.out.println(("Something went wrong: " + ex.getMessage()));
			logger.error("Something went wrong: {}", ex.getMessage());
			System.exit(-1);
		}

		// Infinite REPL loop
		Shell.getInstance().repl();
	}

	private static void shutdown(){

		// Stop automatic control unit (if running mode is AUTO)
		try {
			ControlLogic.shutdown();
			logger.info("Automatic control unit shutdown completed");
		} catch (ControlException ex) {
			logger.error("Automatic control unit failed to gracefully stop: {}", ex.getMessage());
		}

		// Close database connection
		try {
			DataManager.close();
			logger.info("Database disconnection completed");
		} catch (PersistenceException ex) {
			logger.error("Database connection failed to close: {} " + ex.getMessage());
		}

	}
}