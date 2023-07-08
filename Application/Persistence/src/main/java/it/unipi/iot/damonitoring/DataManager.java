package it.unipi.iot.damonitoring;

import it.unipi.iot.damonitoring.entities.FlowRate;
import it.unipi.iot.damonitoring.entities.Resource;
import it.unipi.iot.damonitoring.entities.WaterLevel;
import it.unipi.iot.damonitoring.exceptions.PersistenceException;

import org.hibernate.HibernateException;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.boot.MetadataSources;
import org.hibernate.boot.registry.StandardServiceRegistryBuilder;

import java.util.Date;
import java.util.List;

public class DataManager {
    private static DataManager instance;
    private final SessionFactory sessionFactory;

    public static DataManager getInstance() throws PersistenceException {
        try {
            if(instance == null || instance.sessionFactory.isClosed()){
                instance = new DataManager();
            }
        }
        catch (HibernateException ex){
            throw new PersistenceException("Impossible connect to database: " + ex.getMessage());
        }
        return instance;
    }

    public static void close() throws PersistenceException{
        try {
            if(instance != null && instance.sessionFactory.isOpen()){
                instance.sessionFactory.close();
                instance = null;
            }
        }
        catch (HibernateException ex){
            throw new PersistenceException(ex.getMessage());
        }
    }

    private DataManager() {
        sessionFactory = new MetadataSources(
                new StandardServiceRegistryBuilder()
                    .configure()
                    .build())
                .buildMetadata()
                .buildSessionFactory();
    }

    private Object record(Object record) throws PersistenceException {
        try (Session session = sessionFactory.openSession()){
            session.beginTransaction();
            Object id = session.save(record);
            session.getTransaction().commit();
            return id;
        }
        catch (HibernateException | IllegalStateException ex){
            throw new PersistenceException(ex.getMessage());
        }
    }

    /**
     * Store a new flow rate measurement
     * @param flowChannel Channel identifier
     * @param timestamp Timestamp of measurement
     * @param value Value of measurement
     * @return FlowRate object stored in the database
     * @throws PersistenceException In case of failure of the operation
     */
    public FlowRate recordFlowRate(String flowChannel, Date timestamp, Float value) throws PersistenceException {
        FlowRate flowRate = new FlowRate();
        flowRate.setRecordId(0);
        flowRate.setFlowChannel(flowChannel);
        flowRate.setValue(value);
        flowRate.setTimestamp((timestamp == null)?new Date():timestamp);

        Integer recordId = (Integer) record(flowRate);
        flowRate.setRecordId(recordId);

        return flowRate;
    }

    /**
     * Store a new flow rate measurement of timestamp roughly now
     * @param flowChannel Channel identifier
     * @param value Value of measurement
     * @return FlowRate object stored in the database
     * @throws PersistenceException In case of failure of the operation
     */
    public FlowRate recordFlowRate(String flowChannel, Float value) throws PersistenceException {
        return recordFlowRate(flowChannel, new Date(), value);
    }

    /**
     * Store a new water level measurement
     * @param value Value of measurement
     * @param timestamp Timestamp of measurement
     * @return WaterLevel object stored in the database
     * @throws PersistenceException In case of failure of the operation
     */
    public WaterLevel recordWaterLevel(Float value, Date timestamp) throws PersistenceException{
        WaterLevel waterLevel = new WaterLevel();
        waterLevel.setRecordId(0);
        waterLevel.setValue(value);
        waterLevel.setTimestamp((timestamp == null)?new Date():timestamp);

        Integer recordId = (Integer) record(waterLevel);
        waterLevel.setRecordId(recordId);

        return waterLevel;
    }

    /**
     * Store a new water level measurement of timestamp roughly now
     * @param value Value of measurement
     * @return WaterLevel object stored in the database
     * @throws PersistenceException In case of failure of the operation
     */
    public WaterLevel recordWaterLevel(Float value) throws PersistenceException{
        return recordWaterLevel(value, new Date());
    }

    /**
     * Load the last flow rate measurement stored in the database of the given channel
     * @param flowChannel Channel used to filter the data
     * @return FlowRate object containing requested information
     * @throws PersistenceException In case of failure of the operation
     */
    public FlowRate retrieveFlowRate(String flowChannel) throws PersistenceException {
        try(Session session = sessionFactory.openSession()){
            return (FlowRate) session
                    .createQuery("from FlowRate where flowChannel = :flowChannel order by timestamp desc")
                    .setParameter("flowChannel", flowChannel)
                    .setMaxResults(1)
                    .uniqueResult();
        }
        catch (HibernateException | IllegalStateException ex){
            throw new PersistenceException(ex.getMessage());
        }
    }

    /**
     * Load the sum of the last flow rate of out channels
     * @return The sum of the last outflow measurements
     * @throws PersistenceException In case of failure of the operation
     */
    public Float retrieveOutFlowRate() throws PersistenceException{
        try(Session session = sessionFactory.openSession()){
            return session.createQuery(
                    "select sum(value) from FlowRate where recordId in (select max(recordId) from FlowRate where " +
                            "flowChannel != 'inflow' group by flowChannel)",
                            Double.class).uniqueResult().floatValue();
        }
        catch (HibernateException | IllegalStateException ex){
            throw new PersistenceException(ex.getMessage());
        }
    }

    /**
     * Load the last water level measurement stored in the database
     * @return WaterLevel object containing requested information
     * @throws PersistenceException In case of failure of the operation
     */
    public WaterLevel retrieveWaterLevel() throws PersistenceException{
        try(Session session = sessionFactory.openSession()){
            return (WaterLevel) session
                    .createQuery("from WaterLevel order by timestamp desc")
                    .setMaxResults(1)
                    .uniqueResult();
        }
        catch (HibernateException | IllegalStateException ex){
            throw new PersistenceException(ex.getMessage());
        }
    }

    /**
     * Add the list of resources in the central registry
     * @param resources List of resources to register
     * @throws PersistenceException In case of failure of the operation
     */
    public void addResources(List<Resource> resources) throws PersistenceException {
        try(Session session = sessionFactory.openSession()){
            session.beginTransaction();
            for(Resource resource: resources){
                session.saveOrUpdate(resource);
            }
            session.getTransaction().commit();
        }
        catch (HibernateException | IllegalStateException ex){
            throw new PersistenceException(ex.getMessage());
        }
    }

    /**
     * Retrieve the URI of the requested resource
     * @param resName Resource name used to identify the target resource
     * @return URI of resource
     * @throws PersistenceException In case of failure of the operation
     */
    public Resource lookupResource(String resName) throws PersistenceException{
        try(Session session = sessionFactory.openSession()){
            return session.get(Resource.class, resName);
        }
        catch (HibernateException | IllegalStateException ex){
            throw new PersistenceException(ex.getMessage());
        }
    }

    /**
     * Load all "gate" resource available in the database
     * @return List of "gate" resources
     * @throws PersistenceException In case of failure of the operation
     */
    public List<Resource> gateResources() throws PersistenceException{
        try(Session session = sessionFactory.openSession()){
            return session.createQuery("from Resource where type = 'gate'", Resource.class).list();
        }
        catch (HibernateException | IllegalStateException ex){
            throw new PersistenceException(ex.getMessage());
        }
    }

    /**
     * Load all "alarm" resource available in the database
     * @return List of "alarm" resources
     * @throws PersistenceException In case of failure of the operation
     */
    public List<Resource> alarmResources() throws PersistenceException{
        try(Session session = sessionFactory.openSession()){
            return session.createQuery("from Resource where type = 'alarm'", Resource.class).list();
        }
        catch (HibernateException | IllegalStateException ex){
            throw new PersistenceException(ex.getMessage());
        }
    }

    /**
     * Set the value of the specified resource
     * @param resName Resource name used to identify the target resource
     * @param value Value to assign to the resource
     * @throws PersistenceException In case of failure of the operation
     */
    public void setResourceValue(String resName, Integer value) throws PersistenceException {
        try(Session session = sessionFactory.openSession()){
            session.beginTransaction();
            session.createQuery("UPDATE Resource r SET r.value = :value WHERE r.name = :resName")
                    .setParameter("value", value)
                    .setParameter("resName", resName)
                    .executeUpdate();
            session.getTransaction().commit();

        }
        catch (HibernateException | IllegalStateException ex){
            ex.printStackTrace();
            throw new PersistenceException(ex.getMessage());
        }
    }

    /**
     * Retrieve the last value of the resource stored in the database, might be not synchronized with the actual
     * value of the resource
     * @param resName Resource name used to identify the target resource
     * @return Value of the resource
     * @throws PersistenceException In case of failure of the operation
     */
    public Integer getResourceValue(String resName) throws PersistenceException {
        try(Session session = sessionFactory.openSession()){
            return session.get(Resource.class, resName).getValue();
        }
        catch (NullPointerException ex){
            throw new PersistenceException("Resource unavailable");
        }
        catch (HibernateException | IllegalStateException ex){
            throw new PersistenceException(ex.getMessage());
        }
    }
}
