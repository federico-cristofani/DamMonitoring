package it.unipi.iot.damonitoring.controller.user.commands;

import java.util.concurrent.Callable;

interface BaseCommand extends Callable<Integer> { }
