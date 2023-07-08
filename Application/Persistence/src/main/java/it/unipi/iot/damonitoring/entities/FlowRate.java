package it.unipi.iot.damonitoring.entities;

import javax.persistence.*;
import java.util.Date;

@Entity
@Table(name="flowRate")
public class FlowRate {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Integer recordId;
    @Column(name = "flowChannel")
    private String flowChannel;
    @Column(name = "timestamp")
    private Date timestamp;
    @Column(name = "value")
    private Float value;

    public Integer getRecordId() {
        return recordId;
    }

    public void setRecordId(Integer recordId) {
        this.recordId = recordId;
    }

    public String getFlowChannel() {
        return flowChannel;
    }

    public void setFlowChannel(String flowChannel) {
        this.flowChannel = flowChannel;
    }

    public Date getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(Date timestamp) {
        this.timestamp = timestamp;
    }

    public Float getValue() {
        return value;
    }

    public void setValue(Float value) {
        this.value = value;
    }

    @Override
    public String toString() {
        return "FlowRate{" +
                "recordId=" + recordId +
                ", flowChannel='" + flowChannel + '\'' +
                ", timestamp=" + timestamp +
                ", value=" + value +
                '}';
    }
}
