package it.unipi.iot.damonitoring.entities;

import javax.persistence.*;

@Entity
@Table(name = "resource")
public class Resource {

    @Id
    @Column(name = "name")
    String name;
    @Column(name = "type")
    String type;
    @Column(name = "description")
    String description;
    @Column(name = "tag")
    String tag;
    @Column(name = "uri")
    String uri;
    @Column(name = "value")
    Integer value;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String getTag() {
        return tag;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }

    public String getUri() {
        return uri;
    }

    public void setUri(String uri) {
        this.uri = uri;
    }

    public Integer getValue() {
        return value;
    }

    public void setValue(Integer value) {
        this.value = value;
    }

    @Override
    public String toString() {
        return "Resource{" +
                "name='" + name + '\'' +
                ", type='" + type + '\'' +
                ", description='" + description + '\'' +
                ", tag='" + tag + '\'' +
                ", uri='" + uri + '\'' +
                ", value=" + value +
                '}';
    }
}
