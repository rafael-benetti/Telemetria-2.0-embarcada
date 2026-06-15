FROM node:20-alpine

RUN apk add --no-cache tzdata

WORKDIR /app
COPY server.js /app/server.js

ENV PORT=8080
ENV MQTT_HOST=mqtt
ENV MQTT_PORT=1883
ENV TZ=America/Sao_Paulo

EXPOSE 8080
CMD ["node", "server.js"]
