import {
  boolean,
  integer,
  pgTable,
  timestamp,
  varchar
} from 'drizzle-orm/pg-core';

export const messages = pgTable('messages', {
  id: integer().primaryKey().generatedAlwaysAsIdentity(),
  title: varchar({ length: 128 }),
  content: varchar({ length: 500 }),
  authorId: integer('author_id').references(() => users.id, {
    onDelete: 'set null'
  }),
  deleted: boolean().notNull().default(false),
  createdAt: timestamp('created_at').defaultNow(),
  updatedAt: timestamp('updated_at').$onUpdate(() => new Date())
});

export const users = pgTable('users', {
  id: integer().primaryKey().generatedAlwaysAsIdentity(),
  username: varchar({ length: 20 }).unique().notNull(),
  bio: varchar({ length: 500 }),
  readonly: boolean().notNull().default(false),
  isAdmin: boolean('is_admin').notNull().default(false),
  createdAt: timestamp('created_at').defaultNow()
});

export const reports = pgTable('reports', {
  id: integer().primaryKey().generatedAlwaysAsIdentity(),
  messageId: integer('message_id')
    .notNull()
    .references(() => messages.id, { onDelete: 'cascade' }),
  description: varchar({ length: 1000 }).notNull(),
  sourceIP: varchar('source_ip', { length: 45 }),
  sourceUserAgent: varchar('source_user_agent', { length: 300 }),
  sourceUserId: integer('source_user_id').references(() => users.id),
  createdAt: timestamp('created_at').defaultNow()
});