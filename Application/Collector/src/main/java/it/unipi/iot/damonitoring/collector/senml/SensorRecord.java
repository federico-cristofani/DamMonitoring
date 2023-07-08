package it.unipi.iot.damonitoring.collector.senml;

import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.List;

public class SensorRecord {
    @JsonProperty("bu")
    private String baseUnit;
    @JsonProperty("e")
    private List<Measurement> measurements;

    public String getBaseUnit() {
        return baseUnit;
    }

    public void setBaseUnit(String baseUnit) {
        this.baseUnit = baseUnit;
    }

    public List<Measurement> getMeasurements() {
        return measurements;
    }

    public void setMeasurements(List<Measurement> measurements) {
        this.measurements = measurements;
    }

    @Override
    public String toString() {
        return "{" +
                "baseUnit:'" + baseUnit + '\'' +
                ", measurements:" + measurements +
                '}';
    }
}