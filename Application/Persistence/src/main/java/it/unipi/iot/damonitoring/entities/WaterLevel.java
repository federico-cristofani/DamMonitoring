package it.unipi.iot.damonitoring.entities;

import javax.persistence.*;
import java.util.Date;

@Entity
@Table(name = "waterLevel")
public class WaterLevel {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    @Column(name = "recordId")
    private Integer recordId;

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
        return "WaterLevel{" +
                "recordId=" + recordId +
                ", timestamp=" + timestamp +
                ", value=" + value +
                '}';
    }
}
