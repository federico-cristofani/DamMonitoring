package it.unipi.iot.damonitoring.collector.senml;

import com.fasterxml.jackson.annotation.JsonProperty;

public class Measurement{
    @JsonProperty("n")
    private String name;
    @JsonProperty("v")
    private Float value;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public Float getValue() {
        return value;
    }

    public void setValue(Float value) {
        this.value = value;
    }

    @Override
    public String toString() {
        return "{" +
                "name:'" + name + '\'' +
                ", value:" + value +
                '}';
    }
}