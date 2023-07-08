package it.unipi.iot.damonitoring.controller.control.enumerates;

public enum OperativeMode {
    AUTO, MANUAL;
    public OperativeMode not(){
        return this.equals(AUTO) ? MANUAL:AUTO;
    }
}
